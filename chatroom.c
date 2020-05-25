#include"chatroom.h"
#include"link.h"
#include<stdio.h>
#include<sqlite3.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<time.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<fcntl.h>

char *getChatId()
{
	int fd;
	char cid[20];
	if((fd=open(CHAT_ID_FILE,O_CREAT|O_RDWR,0644))==-1)
	{
		perror("open error");
		exit(1);
	}
	//read cid
	memset(oldcid,0,sizeof(oldcid));
	read(fd,oldcid,20);
	sprintf(cid,"%d",atoi(oldcid)+1);
	lseek(fd,SEEK_SET,0);
	write(fd,cid,strlen(cid));
	close(fd);
	return oldcid;
}

int do_register(int acceptfd,Msg* msg,sqlite3* db)
{
	char *errmsg;
	char sql[128]={};
	static char cid[20];
	printf("do_register\n");
	memset(msg->data,0,1024);
	strcpy(cid,getChatId());
	msg->usr.id=atoi(cid);
	sprintf(sql,"insert into chat_user values(NULL,'%s','%s','%s','%s',datetime('now'))",msg->usr.name,msg->usr.sex,cid,msg->usr.passwd);

	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		printf("%s",errmsg);
		strcpy(msg->data,"register error!");
	}
	else
	{
		printf("client register successful!\n");
		strcpy(msg->data,"register success!");
	}
	if(write(acceptfd,msg,sizeof(Msg))<0)
	{
		perror("write error");
		exit(1);
	}
	return 0;
}
int do_login(int acceptfd,Msg* msg,sqlite3* db)
{
	char cid[20]={};
	char sql[128];
	char *errmsg;
	int nrow;
	int ncloumn;
	int **resultp;
	memset(msg->data,0,1024);
	sprintf(sql,"select nickname from chat_user where cid='%d' and passwd='%s';",msg->usr.id,msg->usr.passwd);
	printf("%s\n",sql);
	if((sqlite3_get_table(db,sql,&resultp,&nrow,&ncloumn,&errmsg))!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
		exit(1);
	}

	sprintf(cid,"%d",msg->usr.id);

	if(nrow==1)
	{
		memset(sql,0,sizeof(sql));
		sprintf(sql,"insert into on_line(id,nickname,sockfd)values('%d','%s','%d')",atoi(cid),resultp[1],acceptfd);
		sqlite3_exec(db,sql,NULL,NULL,&errmsg);
		strcpy(msg->data,"OK");
		write(acceptfd,msg,sizeof(Msg));
		return 1;
	}
	else if(nrow==0)
	{
		strcpy(msg->data,"usr or passwd wrong!");
		write(acceptfd,msg,sizeof(Msg));
		return -1;
	}

	return 0;
}
int do_sendMsg(int acceptfd,Msg* msg,sqlite3* db)
{
	char sql[128];
	char *errmsg;
	int nrow;
	int ncloumn;
	int **resultp;
	int tarsockfd;
	msg->type=3;
	sprintf(sql,"select sockfd from on_line where id='%d'",msg->to);
	if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncloumn,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
	}
	tarsockfd=atoi(resultp[1]);
	printf("tar:%d\n",tarsockfd);
	printf("to:%d\n",msg->to);
	printf("from:%d\n",msg->from);
	printf("data:%s\n",msg->data);
	write(tarsockfd,msg,sizeof(Msg));
	memset(sql,0,128);
	sprintf(sql,"insert into chat_history(from_id,to_id,data,ctime)values('%d','%d','%s',datetime('now'))",msg->from,msg->to,msg->data);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
		exit(1);
	}
	return 0;
}
int do_broadcast(int acceptfd,Msg* msg,sqlite3* db)
{
	char sql[128];
	char *errmsg;
	int nrow;
	int ncloumn;
	int i=0,len=0;
	int **resultp;
	int tarsockfd[100];
	printf("data:%s\n",msg->data);
	sprintf(sql,"insert into broadcast_history(from_id,data,ctime)values('%d','%s',datetime('now'))",msg->from,msg->data);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
	}
	memset(sql,0,128);
	sprintf(sql,"select sockfd from on_line where id!='%d'",msg->from);
	if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncloumn,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
	}

	for(i=0;i<=nrow-1;i++)
	{
		tarsockfd[i]=atoi(resultp[i+1]);
		write(tarsockfd[i],msg,sizeof(Msg));
	}

	return 0;
}

int do_broadcast_history(int acceptfd,Msg* msg,sqlite3* db)
{
	char sql[128];
	char *errmsg;
	int nrow;
	int ncloumn;
	int i=0,j=0;
	int **resultp;
	char buf[50];
	sprintf(sql,"select * from broadcast_history;");
	if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncloumn,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
	}
	for(i=0;i<=nrow;i++)
	{
		for(j=0;j<ncloumn;j++)
		{
			memset(buf,0,sizeof(buf));
			sprintf(buf,"%s\t\t",resultp[ncloumn*i+j]);
			strcat(msg->data,buf);
			if(j==ncloumn-1)
			{
				strcat(msg->data,"\n");
			}
		}
	}
	printf("row:%d clo:%d\n",nrow,ncloumn);
	printf("data:%s\n",msg->data);
	write(acceptfd,msg,sizeof(Msg));
	return 0;
}

int do_history(int acceptfd,Msg* msg,sqlite3* db)
{
	printf("do_history\n");
	int row,col;
	int i=0,j=0;
	char **resultp;
	char *errmsg;
	char sql[128];
	char p[50];
	memset(msg->data,0,1024);
	sprintf(sql,"select * from chat_history where from_id= '%d'and to_id='%d';",msg->from,msg->to);
	printf("%s\n",sql);
	if(sqlite3_get_table(db,sql,&resultp,&row,&col,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
	}
	printf("row=%d,col=%d\n",row,col);

	for(i=0;i<=row;i++)
	{
		for(j=0;j<col;j++)
		{
			sprintf(p,"%s\t",resultp[col*i+j]);
			if(j==3)
			{
				strcat(p,"\n");
			}
			strcat(msg->data,p);
			memset(p,0,50);
		}
	}
	printf("%s\n",msg->data);
	if(write(acceptfd,msg,sizeof(Msg))<0)
	{
		perror("write error!");
		exit(1);
	}
	return 0;
}
int do_list(int acceptfd,Msg* msg,sqlite3* db)
{
	char sql[128];
	char buf[50];
	char *errmsg;
	int nrow;
	int ncloumn;
	int i=0,j=0;
	int **resultp;
	sprintf(sql,"select nickname from on_line");
	if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncloumn,&errmsg)!=SQLITE_OK)
	{
		perror("list error");
		exit(1);
	}
	printf("row:%d clo:%d\n",nrow,ncloumn);
	for(i=1;i<=nrow;i++)
	{
		//printf("%s \n",resultp[i]);
		sprintf(buf,"%s ",resultp[i]);
		strcat(msg->data,buf);
	}
	printf("data:%s\n",msg->data);
	write(acceptfd,msg,sizeof(Msg));
	printf("---\n");

	return 0;
}

int do_create_group(int acceptfd,Msg* msg,sqlite3 *db)
{
	msg->type=9;
	char *errmsg;
	char sql[128]={};
	char buf[100]={};
	char id[100]={};
	int i=0;
	memset(msg->data,0,1024);
	while(1)
	{
		memset(buf,0,sizeof(buf));
		sprintf(buf,"%d|",msg->usr.member[i]);
		strcat(id,buf);
		i++;
		if(msg->usr.member[i]==0)
		{
			break;
		}
	}
	sprintf(sql,"insert into chat_group values('%s','%s')",msg->usr.group_name,id);
	printf("%s\n",sql);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		printf("%s",errmsg);
		strcpy(msg->data,"create group error!");
	}
	else
	{
		strcpy(msg->data,"create group success!");
	}
	if(write(acceptfd,msg,sizeof(Msg))<0)
	{
		perror("write error");
		exit(1);
	}
	return 0;
}

int do_group_chat(int acceptfd,Msg *msg,sqlite3* db)
{
	char sql[128];
	char *errmsg;
	int nrow;
	int ncloumn;
	int **resultp;
	char buf[100];
	char *p=NULL;
	char *s=NULL;
	char id[10];
	msg->type=10;
	printf("do_chat_group\n");
	sprintf(sql,"insert into group_history(group_name,fromid,data,ctime)values('%s','%d','%s',datetime('now'))",msg->usr.group_name,msg->from,msg->data);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
	}
	memset(sql,0,128);
	sprintf(sql,"select id from chat_group where name='%s'",msg->usr.group_name);
	printf("%s\n",sql);
	if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncloumn,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
	}
	printf("row:%d,clo:%d\n",nrow,ncloumn);
	printf("%s\n",resultp[1]);
	sprintf(buf,"%s",resultp[1]);
	p=buf;
	while(*p)
	{
		s=id;
		while(*p!='|')
		{
			*s=*p;
			s++;
			p++;
		}
		*s=0;
		p++;
		printf("%s\n",id);
		memset(sql,0,128);
		nrow=0;
		ncloumn=0;
		memset(resultp,0,sizeof(resultp));
		sprintf(sql,"select sockfd from on_line where id='%d'",atoi(id));
		printf("%s\n",sql);
		if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncloumn,&errmsg)!=SQLITE_OK)
		{
			printf("%s\n",errmsg);
		}
		printf("%d\n",atoi(resultp[1]));
		write(atoi(resultp[1]),msg,sizeof(msg));
		memset(id,0,sizeof(id));
		memset(s,0,sizeof(s));
	}

	return 0;
}

int do_group_history(int acceptfd,Msg* msg,sqlite3* db)
{
	int row,col;
	int i=0,j=0;
	char **resultp;
	char *errmsg;
	char sql[128];
	char p[50];
	memset(msg->data,0,1024);
	sprintf(sql,"select * from group_history where group_name= '%s';",msg->usr.group_name);
	printf("%s\n",sql);
	if(sqlite3_get_table(db,sql,&resultp,&row,&col,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
	}
	printf("row=%d,col=%d\n",row,col);

	for(i=0;i<=row;i++)
	{
		for(j=0;j<col;j++)
		{
			sprintf(p,"%s\t",resultp[col*i+j]);
			if(j==3)
			{
				strcat(p,"\n");
			}
			strcat(msg->data,p);
			memset(p,0,50);
		}
	}
	printf("%s\n",msg->data);
	if(write(acceptfd,msg,sizeof(Msg))<0)
	{
		perror("write error!");
		exit(1);
	}
	return 0;
}
int do_send_file_U(int acceptfd,Msg* msg,sqlite3* db)
{
	char sql[128];
	char *errmsg;
	int nrow;
	int ncloumn;
	int **resultp;
	int tarsockfd;
	msg->type=12;
	sprintf(sql,"select sockfd from on_line where id='%d'",msg->to);
	if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncloumn,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
	}
	tarsockfd=atoi(resultp[1]);
	printf("tar:%d\n",tarsockfd);
	printf("to:%d\n",msg->to);
	printf("from:%d\n",msg->from);
	printf("data:%s\n",msg->data);
	write(tarsockfd,msg,sizeof(Msg));
	return 0;
}

int do_send_file_B(int acceptfd,Msg* msg,sqlite3* db)
{
	char sql[128];
	char *errmsg;
	int nrow;
	int ncloumn;
	int i=0,len=0;
	int **resultp;
	int tarsockfd[100];
	msg->type=13;
	printf("data:%s\n",msg->data);
	sprintf(sql,"select sockfd from on_line where id!='%d'",msg->from);
	if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncloumn,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
	}

	for(i=0;i<=nrow-1;i++)
	{
		tarsockfd[i]=atoi(resultp[i+1]);
		write(tarsockfd[i],msg,sizeof(Msg));
	}

	return 0;
}
int do_logout(int acceptfd,Msg* msg,sqlite3* db)
{
	char sql[128];
	char *errmsg;
	sprintf(sql,"delete from on_line where id='%d'",msg->from);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
		exit(1);
	}
	return 0;
}

char *getChatId2()
{
	int fd;
	char aid[20];
	if((fd=open(ADMIN_ID_FILE,O_CREAT|O_RDWR,0644))==-1)
	{
		perror("open error");
		exit(1);
	}
	//read cid
	memset(oldaid,0,sizeof(oldaid));
	read(fd,oldaid,20);
	sprintf(aid,"%d",atoi(oldaid)+1);
	lseek(fd,SEEK_SET,0);
	write(fd,aid,strlen(aid));
	close(fd);
	return oldaid;
}

int do_admin_register(int acceptfd,Msg* msg,sqlite3* db)
{
	char passwd[20];
	char *errmsg;
	char sql[128]={};
	static char aid[20];
	memset(msg->data,0,1024);
	strcpy(aid,getChatId2());
	msg->usr.id=atoi(aid);
	sprintf(sql,"insert into admin values(NULL,'%d','%s',datetime('now'))",atoi(aid),msg->usr.passwd);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		printf("%s",errmsg);
		strcpy(msg->data,"register error!");
	}
	else
	{
		printf("client register successful!\n");
		strcpy(msg->data,"register admin success!");
	}
	if(write(acceptfd,msg,sizeof(Msg))<0)
	{
		perror("write error");
		exit(1);
	}
	return 0;
}

int do_admin_login(int acceptfd,Msg* msg,sqlite3* db)
{
	int aid;
	char passwd[20];
	char cid[20]={};
	char sql[128];
	char *errmsg;
	int nrow;
	int ncloumn;
	int **resultp;
	sprintf(sql,"select * from admin where aid='%d' and passwd='%s';",msg->usr.id,msg->usr.passwd);
	printf("%s\n",sql);
	if((sqlite3_get_table(db,sql,&resultp,&nrow,&ncloumn,&errmsg))!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
		exit(1);
	}

	if(nrow==1)
	{
		strcpy(msg->data,"OK");
		write(acceptfd,msg,sizeof(Msg));
		return 1;
	}
	else if(nrow==0)
	{
		strcpy(msg->data,"usr or passwd wrong!");
		write(acceptfd,msg,sizeof(Msg));
		return -1;
	}
	return 0;
}

int do_kickout_user(int acceptfd,Msg* msg,sqlite3* db)
{
	char *errmsg;
	char sql[128]={};
	sprintf(sql,"delete from chat_user where cid='%d'",msg->usr.id);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		printf("%s",errmsg);
		strcpy(msg->data,"fail to kickout the user");
		write(acceptfd,msg,sizeof(Msg));
		exit(1);
	}
	else
	{
		printf("success!\n");
		strcpy(msg->data,"kickout user success");
		write(acceptfd,msg,sizeof(Msg));
	}
	return 0;
}

int do_client(int acceptfd,sqlite3 *db,LinkList fl)
{
	Msg msg;
	printf("do_client\n");
	while(read(acceptfd,&msg,sizeof(Msg))>0)
	{
		if(msg.type==-1)
		{
			continue;
		}
		printf("type:%d\n",msg.type);
		switch(msg.type)
		{
		case R:
			do_register(acceptfd,&msg,db);
			break;
		case L:
			do_login(acceptfd,&msg,db);
			break;
		case S:
			do_sendMsg(acceptfd,&msg,db);
			break;
		case B:
			do_broadcast(acceptfd,&msg,db);
			break;
		case B_H:
			do_broadcast_history(acceptfd,&msg,db);
			break;
		case H:
			do_history(acceptfd,&msg,db);
			break;
		case LI:
			do_list(acceptfd,&msg,db);
			break;
		case C:
			do_create_group(acceptfd,&msg,db);
			break;
		case G:
			do_group_chat(acceptfd,&msg,db);
			break;
		case G_H:
			do_group_history(acceptfd,&msg,db);
			break;
		case S_F_U:
			do_send_file_U(acceptfd,&msg,db);
			break;
		case S_F_B:
			do_send_file_B(acceptfd,&msg,db);
			break;
		case A_R:
			do_admin_register(acceptfd,&msg,db);
			break;
		case A_L:
			do_admin_login(acceptfd,&msg,db);
			break;
		case A_K:
			do_kickout_user(acceptfd,&msg,db);
			break;
		case O:
			do_logout(acceptfd,&msg,db);
			sbreak;
		}
	}
	printf("client quit!\n");
	close(acceptfd);
	return 0;
}








