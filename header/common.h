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
#define BACKLOG 2
#define BUFSIZE 1024
#define NAMELEN 10
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


#define SELFFD self->fds[0].fd
#define CONNFD self->fds[1].fd


struct thread_info
{
    long int tid;           //thread id of current thread
    struct pollfd fds[2];   //two fds in this 0 for socket 1 for connection
    int run_flag : 1;       //run flag,client is active or not
    int busy_flag : 1;      //busy flag, is client is connected with any other or not
    char name[NAMELEN];     //name of current client
    char conn_name[NAMELEN];            //socket fd of connected client
    struct thread_manage **head;
};

struct thread_manage{
    struct thread_info self;
    struct thread_manage* next;
};

typedef struct thread_info thread_info;
typedef struct thread_manage thread_manage;

typedef enum {
    SUCCESS = 0,
    NULL_PTR,
    SOCK_FAIL,
    BIND_FAIL,
    LISN_FAIL,
    DMA_FAIL,
    NODE_NOT_FOUND,
    NULL_LIST,
    USER_NOT_FOUND,
    BUSY,
    MODE_OUT_OF_RAGE,
    NULL_BYTES,
    TERMINATE_MSG,
    NAME_ADDED,
    REQ_LIST_T,
    BREAK,
    CONT
} r_type;

#define RST_LOG_BFR memset(logMsg, '\0', strlen(logMsg))

extern pthread_mutex_t listMutex ;


extern short run_flag ;
extern short open ;

/*functions*/

#endif



