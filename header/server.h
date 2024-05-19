#include "common.h"

#ifndef SERVER
#define SERVER

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

int check_from_other_side(thread_info *self,char *buff);


#endif