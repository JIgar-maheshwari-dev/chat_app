#ifndef COMMON_H
#define COMMON_H

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8080
#define BACKLOG 10
#define BUFSIZE 1024
#define NAMELEN 30

#define IP "127.0.0.1"
#define TIMEOUT 1000
#define T_TIMEOUT 1000


#define REQ_NAME "SENDNAME"
#define REPLNAME "NAME: "

#define REQ_LIST "getlist"
#define CONN_REQ "connect "

#define CONN_FAIL1 "USER_BUSY"
#define CONN_FAIL2 "USER_NOT_FOUND"
#define SAME_USER "NOT ALLOW TO CONNECT WITH YOURSELF"
#define UNDEF_CONN "USER NOT ADDED NAME"
#define NAME_EXIST "ALREADY OTHER HAS SAME NAME ADD NEW "

#define CONN_SUCC "Connection successfull...."

#define ENTERNAME "Enter Your Name : "
#define TERMINATE "EXIT"
#define CHAT_OVER "BYE"
#define UNDEF "unknown"
#define ILLG_CMD "ILLIGAL CMD "

#define FRAME_START "`"
#define FRAME_END "`"
#define MSG_START "&"
#define MSG_END "&"
#define DEFRAME "`&"

#endif