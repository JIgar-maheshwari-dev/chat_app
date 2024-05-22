#include "../header/common.h"

#ifndef SERVER
#define SERVER

struct thread_info
{
    long int tid;           //thread id of current thread
    struct pollfd fds[2];   //two fds in this 0 for socket 1 for connection
    int run_flag ;       //run flag,client is active or not
    int busy_flag ;      //busy flag, is client is connected with any other or not
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

void sig_handler(int sig);

void create_thread_manage(thread_manage *temp, int fd, thread_manage **head);

void *thread_handle(void *arg);

int get_socket_s(int *fd, struct sockaddr_in *addr);

void close_all_thread(thread_manage **);

void join_all(thread_manage **head);

void addLog(FILE *fp, char *log);

void close_all_fd(thread_manage **head);

char **get_list(thread_manage **head,int *number);

char *make_frame(char **list,int n);

void deframe_print(char *frame);

void del_node(thread_info *self);

int check_for_data(thread_info *self,char *buff ) ;

char *get_name(char name[]);

int check_status_after_data_read(int status,thread_info *self);

int check_and_connect(thread_info *self, char* name);

void del_connection(thread_info *self);

thread_info *get_info_with_name(thread_info *self,char *name);

void print_list(thread_manage *head);

int check_name(thread_info *self,char *name);

#define SELFFD self->fds[0].fd
#define CONNFD self->fds[1].fd


#define RST_LOG_BFR memset(logMsg, '\0', strlen(logMsg))
#define ADDLOG addLog(fp, logMsg);

extern pthread_mutex_t listMutex ;
extern char logMsg[BUFSIZE + 100] ;
extern FILE *fp;
extern short run_flag ;
extern short open ;


typedef enum {
    SUCCESS = 0,        //0
    NULL_PTR,           //1
    SOCK_FAIL,          //2
    BIND_FAIL,          //3
    LISN_FAIL,          //4
    DMA_FAIL,           //5
    CONN_WITH_ITSELF,   //6
    NODE_NOT_FOUND,     //7
    NULL_LIST,          //8
    USER_NOT_FOUND,     //9
    FOUND,              //10
    BUSY,               //11
    NULL_BYTES,         //12
    TERMINATE_MSG,      //13
    NAME_ADDED,         //14
    REQ_LIST_T,         //15
    BREAK,              //16
    CONT,               //17
    CONN_REQ_T,         //18
    TERMINATE_MSG_BUSY,  //19
    BYE_T,               //20
    CONN_UND             //21
} r_type;   


#endif