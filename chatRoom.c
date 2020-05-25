#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<string.h>
#include<pthread.h>
#include"chatRoom.h"

int a=0;
int b=0;
int id=0;
char me[50];
Msg msg;

int do_register(int sockfd,Msg *msg)
{
	memset(msg,0,sizeof(Msg));
	msg->type=R;
	printf("input your name:\n");
	scanf("%s",msg->usr.name);
	getchar();
	strcpy(me,msg->usr.name);
	printf("name:%s\n",msg->usr.name);

	printf("input your password:\n");
	scanf("%s",msg->usr.passwd);
	getchar();

	printf("input your sex:(1:man 0:woman)\n");
	scanf("%s",msg->usr.sex);
	if(write(sockfd,msg,sizeof(Msg))<0)
	{
		perror("write error");
		exit(1);
	}

	return 1;
}

int do_login(int sockfd,Msg *msg)
{
	memset(msg,0,sizeof(Msg));
	msg->type=L;
	printf("Input your chatID:");
	scanf("%d",&(msg->usr.id));
	id=msg->usr.id;
	getchar();
	id=msg->usr.id;
	printf("Input your password:");
	scanf("%s",msg->usr.passwd);
	getchar();

	if(write(sockfd,msg,sizeof(Msg))<0)
	{
		perror("write error");
		exit(1);
	}
	sleep(1);
	if(a==1)
	{
		return 1;
	}
	return 0;
}

int do_sendMsg(int sockfd,Msg *msg)
{
	char buf[100];
	memset(msg,0,sizeof(Msg));
	msg->type=S;
	msg->from=id;
	printf("from:%d\n",msg->from);
	printf("send to:");
	scanf("%d",&(msg->to));
	printf("Input the data:");
	scanf("%s",buf);
	if(strcmp(buf,":)")==0)
	{
		sprintf(msg->data,"%d make a smiling face",id);
	}
	else
	{
		sprintf(msg->data,buf);
	}
	if(write(sockfd,msg,sizeof(Msg))<0)
	{
		perror("write error");
		exit(1);
	}
	return 0;
}

int do_broadcast(int sockfd,Msg *msg)
{
	memset(msg,0,sizeof(Msg));
	msg->type=B;
	msg->from=id;
	printf("input the message:");
	scanf("%s",msg->data);
	write(sockfd,msg,sizeof(Msg));
	return 0;
}

int do_broadcase_history(int sockfd,Msg *msg)
{
	memset(msg,0,sizeof(Msg));
	msg->type=B_H;
	write(sockfd,msg,sizeof(Msg));
	return 0;
}

int do_history(int sockfd,Msg *msg)
{
	memset(msg,0,sizeof(Msg));
	msg->type=H;
	printf("with who?");
	scanf("%d",&(msg->to));
	msg->from=id;
	printf("%d\n",msg->from);
	if(write(sockfd,msg,sizeof(Msg))<0)
	{
		perror("write error");
		exit(1);
	}

	return 0;
}

int do_list(int sockfd,Msg* msg)
{
	memset(msg,0,sizeof(Msg));
	msg->type=LI;
	if(write(sockfd,msg,sizeof(Msg))<0)
	{
		perror("write error");
		exit(1);
	}

	return 0;
}

int do_create_group(int sockfd,Msg *msg)
{
	int i=0;
	memset(msg,0,sizeof(Msg));
	msg->type=C;
	printf("input the proup_name:\n");
	scanf("%s",msg->usr.group_name);
	printf("Member invitation('0'over)\n");
	while(1)
	{
		scanf("%d",&(msg->usr.member[i]));
		if((msg->usr.member[i]==0)|(i==19))
		{
			break;
		}
		i++;
	}
	write(sockfd,msg,sizeof(Msg));

	return 0;
}

int do_group_chat(int sockfd,Msg *msg)
{

	memset(msg,0,sizeof(Msg));
	msg->type=G;
	printf("input the group name\n");
	scanf("%s",msg->usr.group_name);
	getchar();
	msg->from=id;
	printf("input the message:");
	scanf("%s",msg->data);
	write(sockfd,msg,sizeof(Msg));
	return 0;
}

int do_group_history(int sockfd,Msg *msg)
{
	memset(msg,0,sizeof(Msg));
	msg->type=G_H;
	printf("input the group_name:\n");
	scanf("%s",msg->usr.group_name);
	getchar();
	if(write(sockfd,msg,sizeof(Msg))<0)
	{
		perror("write error");
		exit(1);
	}

	return 0;
}

int do_send_file_U(int sockfd,Msg *msg)
{
	int fd;
	char path[50];
	char buf[100];
	int len=0;
	memset(msg,0,sizeof(Msg));

	msg->type=S_F_U;
	msg->from=id;
	printf("send to who?\n");
	scanf("%d",&msg->to);
	getchar();
	printf("input the path\n");
	scanf("%s",path);
	getchar();
	printf("path:%s\n",path);
	fd=open(path,O_RDONLY);
	if(fd==-1)
	{
		perror("open error");
		exit(1);
	}

	while(1)
	{
		memset(buf,0,sizeof(buf));
		len=read(fd,buf,sizeof(buf));
		//printf("len:%d\n",len);
		strcat(msg->data,buf);
		if(len<sizeof(buf))
		{
			printf("%s\n",msg->data);
			write(sockfd,msg,sizeof(Msg));
			break;
		}
	}
	close(fd);
	return 0;
}

int do_send_file_B(int sockfd,Msg* msg)
{
	int fd;
	char path[50];
	char buf[100];
	int len=0;
	memset(msg,0,sizeof(Msg));
	msg->type=S_F_B;
	msg->from=id;
	printf("input the path\n");
	scanf("%s",path);
	getchar();
	fd=open(path,O_RDONLY);
	if(fd==-1)
	{
		perror("open error");
		exit(1);
	}

	while(1)
	{
		memset(buf,0,sizeof(buf));
		len=read(fd,buf,sizeof(buf));
		strcat(msg->data,buf);
		if(len<sizeof(buf))
		{
			printf("%s\n",msg->data);
			write(sockfd,msg,sizeof(Msg));
			break;
			break;
		}
	}
	return 0;
}

int do_admin_register(int sockfd,Msg *msg)
{
	memset(msg,0,sizeof(Msg));
	msg->type=A_R;
	printf("input your passwd\n");
	scanf("%s",msg->usr.passwd);
	getchar();
	if(write(sockfd,msg,sizeof(Msg))<0)
	{
		perror("write error");
		exit(1);
	}
	sleep(1);
	return 0;
}

int do_admin_login(int sockfd,Msg *msg)
{
	memset(msg,0,sizeof(Msg));
	msg->type=A_L;
	printf("Input your admin_id:");
	scanf("%d",&(msg->usr.id));
	id=msg->usr.id;
	getchar();
	id=msg->usr.id;
	printf("Input your password:");
	scanf("%s",msg->usr.passwd);
	getchar();

	if(write(sockfd,msg,sizeof(Msg))<0)
	{
		perror("write error");
		exit(1);
	}
	sleep(1);
	if(b==1)
	{
		return 1;
	}
	return 0;
}

void do_kickout_user(int sockfd,Msg *msg)
{
	memset(msg,0,sizeof(Msg));
	msg->type=A_K;
	printf("input the user's id\n");
	scanf("%d",&(msg->usr.id));
	getchar();
	write(sockfd,msg,sizeof(Msg));
	return ;
}

void do_admin(int sockfd,Msg *msg)
{
	int cmd;
	while(1)
	{
		printf("1.register  2.login  3.exit \n");
		scanf("%d",&cmd);
		switch(cmd)
		{
		case 1:
			do_admin_register(sockfd,msg);
			break;
		case 2:
			if(do_admin_login(sockfd,msg)==1)
			{
				goto next2;
			}
			break;
		case 3:
			close(sockfd);
			exit(0);
			break;
		default:
			printf("Invalid data cmd!\n");
		}
	}
next2:
	while(1)
	{
		printf("1.kickout_user  2.Don't speak \n");
		printf("input the cmd\n");
		scanf("%d",&cmd);
		getchar();
		switch(cmd)
		{
		case 1:
			do_kickout_user(sockfd,msg);
			break;
		case 2:
			break;
		}
	}

}

void do_logout(int sockfd,Msg* msg)
{
	memset(msg,0,sizeof(Msg));
	msg->type=O;
	msg->from=id;
	if(write(sockfd,msg,sizeof(Msg))<0)
	{
		perror("write error");
		exit(1);
	}
	printf("bye!\n");
	shutdown(sockfd,SHUT_RDWR);
	exit(0);
}
void *sendMsg(void *arg)
{
	int s_sockfd=*(int *)arg;
	int choose=0;

	while(1)
	{
		printf("********************************************************\n");
		printf("*****   1:register    2:login  3.admin  4.exit  ******\n");
		printf("********************************************************\n");
		printf("Input the choose:");
		scanf("%d",&choose);
		getchar();
		switch(choose)
		{
		case 1:
			do_register(s_sockfd,&msg);
			sleep(1);
			break;
		case 2:
			if(do_login(s_sockfd,&msg)==1)
			{
				goto next;
			}
			break;
		case 3:
			do_admin(s_sockfd,&msg);
			break;
		case 4:
			close(s_sockfd);
			exit(0);
			break;
		default:
			printf("Invalid data cmd!\n");
		}
	}
next:
	while(1)
	{
		printf("*******************************************************************\n");
		printf("***1:sendMSg  2:broadcast    3.broadcast_history 4.user_history  **\n");
		printf("***5.list     6.createGroup  7.group_chat        8.group_history **\n");
		printf("***9.send_file_U 10.send_file_B  11.logout **\n");
		printf("*******************************************************************\n");
		printf("Input the cmd\n");
		scanf("%d",&choose);
		getchar();
		switch(choose)
		{
		case 1:
			do_sendMsg(s_sockfd,&msg);
			break;
		case 2:
			do_broadcast(s_sockfd,&msg);
			break;
		case 3:
			do_broadcase_history(s_sockfd,&msg);
			break;
		case 4:
			do_history(s_sockfd,&msg);
			break;
		case 5:
			do_list(s_sockfd,&msg);
			break;
		case 6:
			do_create_group(s_sockfd,&msg);
			break;
		case 7:
			do_group_chat(s_sockfd,&msg);
			break;
		case 8:
			do_group_history(s_sockfd,&msg);
			break;
		case 9:
			do_send_file_U(s_sockfd,&msg);
			break;
		case 10:
			do_send_file_B(s_sockfd,&msg);
			break;
		case 11:
			do_logout(s_sockfd,&msg);
			break;
		default:
			printf("Invalid data cmd\n");
		}
	}
}

void *readMsg(void *arg)
{
	int s_sockfd=*(int *)arg;
	while(1)
	{
		memset(&msg,0,sizeof(Msg));
		read(s_sockfd,&msg,sizeof(Msg));
		if(msg.type==R)
		{
			printf("%s  your id is:%d\n",msg.data,msg.usr.id);
		}
		if(msg.type==L)
		{
			if(strcmp(msg.data,"OK")==0)
			{
				printf("Login OK!\n");
				a=1;
			}
			else
			{
				printf("%s\n",msg.data);
			}
		}
		if(msg.type==S)
		{
			printf("recv msg from:%d,data:%s\n",msg.from,msg.data);
		}
		if(msg.type==B)
		{
			printf("recv broadcast from:%d,data:%s\n",msg.from,msg.data);
		}
		if(msg.type==B_H)
		{
			printf("the broadcast history is:\n");
			printf("%s\n",msg.data);
		}
		if(msg.type==H)
		{
			printf("with %d history :%s\n",msg.to,msg.data);
		}
		if(msg.type==LI)
		{
			printf("list:%s\n",msg.data);
		}
		if(msg.type==C)
		{
			printf("%s\n",msg.data);
		}
		if(msg.type==G)
		{
			printf("recv group:%s chat from:%d data:%s\n",msg.usr.group_name,msg.from,msg.data);
		}
		if(msg.type==G_H)
		{
			printf("the %s: history is:\r%s\n",msg.usr.group_name,msg.data);
		}
		if(msg.type==S_F_U)
		{
			printf("vecv file from:%d,data:%s\n",msg.from,msg.data);
		}
		if(msg.type==S_F_B)
		{
			printf("recv broadcast file from:%d,data:%s\n",msg.from,msg.data);
		}
		if(msg.type==A_R)
		{
			printf("%s  your id is:%d\n",msg.data,msg.usr.id);
		}
		if(msg.type==A_L)
		{
			if(strcmp(msg.data,"OK")==0)
			{
				printf("Login OK!\n");
				b=1;
			}
			else
			{
				printf("%s\n",msg.data);
			}
		}
		if(msg.type==A_K)
		{
			printf("%s\n",msg.data);
		}
	}
}
