#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<string.h>
#include<arpa/inet.h>
#include<sqlite3.h>
#include<signal.h>
#include<time.h>
#include<sys/types.h>
#include<netinet/in.h>
#include"link.h"
#include"chatroom.h"

LinkList fl={0,NULL,NULL};
int main()
{
	int n;
	sqlite3 *db;
	if(sqlite3_open(DATABASE,&db)!=SQLITE_OK)
	{
		printf("%s\n",sqlite3_errmsg(db));
		return -1;
	}
	else
	{
		printf("open success!\n");
	}
	char *errmsg;
	if(sqlite3_exec(db,"delete from on_line;",NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
	}

	int s_sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(s_sockfd==-1)
	{
		perror("socket error");
		exit(1);
	}

	struct sockaddr_in s_addr;
	s_addr.sin_family=AF_INET;
	s_addr.sin_port=htons(PORT);
	s_addr.sin_addr.s_addr=inet_addr(IP);

	int opt=1;
	setsockopt(s_sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(int));
	if(bind(s_sockfd,(struct sockaddr*)(&s_addr),sizeof(s_addr))<0)
	{
		perror("bind error");
		exit(1);
	}
	if((listen(s_sockfd,5))<0)
	{
		perror("listen error!");
		exit(1);
	}

	int acceptfd;
	pid_t pid;

	//处理僵尸进程
	signal(SIGCHLD,SIG_IGN);
	struct sockaddr_in c_addr;
	socklen_t addrlen=sizeof(struct sockaddr);

	while(1)
	{
		acceptfd=accept(s_sockfd,(struct sockaddr*)(&c_addr),&(addrlen));
		//printf("main:accept:%d\n",acceptfd);
		if(acceptfd<0)
		{
			perror("fail to accept!");
			exit(1);
		}

		if((pid=fork())<0)
		{
			perror("fail to fork!");
			exit(1);
		}
		else if(pid==0)
		{
			//do_client
			//close(s_sockfd);
			do_client(acceptfd,db,fl);
		}
		else if(pid>0)
		{
			//close(acceptfd);
		}
	}

	return 0;
}
