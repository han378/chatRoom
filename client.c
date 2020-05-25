#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<string.h>
#include<pthread.h>
#include"chatRoom.h"


int main()
{
	int s_sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(s_sockfd==-1)
	{
		perror("socked error");
		exit(1);
	}
	struct sockaddr_in s_addr;
	s_addr.sin_family=AF_INET;
	s_addr.sin_port=htons(PORT);
	s_addr.sin_addr.s_addr=inet_addr(IP);

	if(connect(s_sockfd,(struct sockaddr*)(&s_addr),sizeof(struct sockaddr_in))==-1)
	{
			perror("connect error");
			exit(1);
	}

	pthread_t pid1,pid2;
	if(pthread_create(&pid1,NULL,sendMsg,&s_sockfd)==-1)
	{
		perror("pthread_create1 error");
		exit(1);
	}
	if(pthread_create(&pid2,NULL,readMsg,&s_sockfd)==-1)
	{
		perror("pthread_create2 error");
		exit(1);
	}
	pthread_join(pid1,NULL);
	pthread_join(pid2,NULL);

	return 0;
}
