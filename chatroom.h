#ifndef _CHATROOM__H
#define _CHATROOM__H

#include"chatroom.h"
#include"link.h"
#include<sqlite3.h>

#define PORT 50000
#define IP "172.16.137.116"
#define DATABASE "my.db"
#define CHAT_ID_FILE "id.txt"
#define ADMIN_ID_FILE "admin_id.txt"

#define R 1  //register
#define L 2  //login
#define S 3  //send msg
#define B 4  //broadcast
#define B_H 8  //broadcast history
#define H 5  //user_history
#define LI 6  //list
#define C 9  //create group
#define G 10  //group chat
#define G_H 11 //group history
#define S_F_U 12  //send file to user
#define S_F_B 13  //broadcast send file
#define O 7  //logout
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

char oldcid[20];
char oldaid[20];

int do_admin(sqlite3 *db);
int do_admin_register(int acceptfd,Msg* msg,sqlite3* db);
int do_admin_login(int acceptfd,Msg* msg,sqlite3* db);
char *getChatId2();
char *getChatId();
int do_kickout_user(int acceptfd,Msg* msg,sqlite3* db);
int do_client(int acceptfd,sqlite3 *db,LinkList fl);
int do_register(int acceptfd,Msg* msg,sqlite3* db);
int do_login(int acceptfd,Msg* msg,sqlite3* db);
int do_sendMsg(int acceptfd,Msg* msg,sqlite3* db);
int do_broadcast(int acceptfd,Msg* msg,sqlite3* db);
int do_broadcast_history(int acceptfd,Msg* msg,sqlite3* db);
int do_history(int acceptfd,Msg* msg,sqlite3* db);
int do_list(int acceptfd,Msg* msg,sqlite3* db);
int do_send_file_U(int acceptfd,Msg* msg,sqlite3* db);
int do_send_file_B(int acceptfd,Msg* msg,sqlite3* db);
int do_logout(int acceptfd,Msg* msg,sqlite3* db);
int do_create_group(int sockfd,Msg* msg,sqlite3 *db);
int do_group_chat(int acceptfd,Msg *msg,sqlite3* db);
int do_group_history(int acceptfd,Msg* msg,sqlite3* db);

#endif


