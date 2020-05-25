#ifndef __CHATROOM__H
#define __CHATROOM__H

#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<string.h>
#include<pthread.h>

#define N 32
#define R 1//register R 1
#define L 2   //login
#define S 3   //send msg
#define B 4   //broadcast
#define B_H 8  //broadcast history
#define H 5   //user_history
#define LI 6  //list
#define C 9 //create group
#define G 10 //group chat
#define G_H 11 //group history
#define S_F_U 12 //send file to user
#define S_F_B 13 //broadcast send file
#define O 7   //logout
#define A_R 14
#define A_L 15
#define A_K 16
#define A_N 17

struct info
{
	int id;
	char name[50];
	char group_name[50];
	int member[20];
	char passwd[50];
	char sex[1];
};

typedef struct
{
	char type;
	int to;
	int from;
	char data[1024];
	struct info usr;
}Msg;

#define PORT 50000
#define IP "120.26.186.235"

int do_register(int sockfd,Msg *msg);
int do_login(int sockfd,Msg *msg);
int do_sendMsg(int sockfd,Msg *msg);
int do_broadcast(int sockfd,Msg *msg);
int do_broadcase_history(int sockfd,Msg *msg);
int do_history(int sockfd,Msg *msg);
int do_list(int sockfd,Msg* msg);
int do_create_group(int sockfd,Msg *msg);
int do_group_chat(int sockfd,Msg *msg);
int do_group_history(int sockfd,Msg *msg);
int do_send_file_U(int sockfd,Msg *msg);
int do_send_file_B(int sockfd,Msg* msg);
int do_admin_register(int sockfd,Msg *msg);
int do_admin_login(int sockfd,Msg *msg);
void do_kickout_user(int sockfd,Msg *msg);
void do_admin(int sockfd,Msg *msg);
void do_logout(int sockfd,Msg* msg);
void *sendMsg(void *arg);
void *readMsg(void *arg);

#endif
