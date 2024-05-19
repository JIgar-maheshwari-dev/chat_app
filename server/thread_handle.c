#include "../header/server.h"
int check_for_data(struct pollfd fd,char *buff,char *name); 
char *get_name(char name[]);
int check_status_after_data_read(int status,thread_info *self);

void *thread_handle(void *arg)
{
    thread_info *self = (thread_info *)arg;
    self->tid = pthread_self();
    char buffer[BUFSIZE]; // rec buffer
    char msg[BUFSIZE];    // send buffer

    memset(buffer, '\0', sizeof(buffer));
    memset(msg, '\0', sizeof(msg));

    strcpy(msg, REQ_NAME);

    send(SELFFD, REQ_NAME, strlen(REQ_NAME), 0);

    while ( self->run_flag )
    {
        int status = poll(self->fds, 2, T_TIMEOUT);
        if (status == -1)
        {
            // printf("Poll Error..\n");
            break;
        }
        status = check_for_data(self->fds[0],msg,self->name);  //checking data from self client side
        if( status != SUCCESS)
        {
            status =  check_status_after_data_read(status,self);
            if( BREAK == status )
            {
                break;
            }
            if(CONT == status )
            {
                continue;
            }
        }
        status = check_for_data(self->fds[1],buffer,self->conn_name);  //checking data from connected client side
        if( status != SUCCESS)
        {
            status =  check_status_after_data_read(status,self);
            if( BREAK == status )
            {
                break;
            }
            if(CONT == status )
            {
                continue;
            }
        }
    }
    if( !(self->run_flag) )
    {
        printf("Sending Terminate msg to [ %s ] \n",self->name);
        send(SELFFD,TERMINATE,strlen(TERMINATE),0);
        usleep(1000);
    }
    close(SELFFD);
    pthread_mutex_lock(&listMutex);
    del_node(self);
    pthread_mutex_unlock(&listMutex);
    printf("[ %s ] over....\n", self->name);
    return NULL;
}

int check_for_data(struct pollfd fd,char *buff,char *name) 
{
    r_type r = SUCCESS;
    if(fd.revents & POLLIN )
    {
        //something is to be read from sock_fd
        memset(buff,'\0',sizeof(buff));
        int bytes_read = recv(fd.fd,buff,BUFSIZE,0);
        if( !bytes_read )
        {
            r = NULL_BYTES;
            return r;
        }
        else if( !strcmp(buff,TERMINATE) )
        {
            // printf("Terminate msg fron client [ %s ] \n",name);
            r = TERMINATE_MSG;
            return r;
        }
        else if( !(strncmp(buff,REPLNAME,strlen(REPLNAME))) )
        {
            //client has send its name
            char *name_temp = get_name(buff);
            if(name_temp == NULL )
            {
                r = NULL_PTR;
                return r;
            }
            printf("[ %s ] Name added \n", name_temp );
            // memset(name,'\0',strlen(UNDEF));
            strcpy(name,name_temp);
            r = NAME_ADDED;
            return r;
        }
        else if( !strcmp(buff,REQ_LIST) )
        {
            //client requested the list
            printf("Client requested a list of users \n");
            r = REQ_LIST_T;
            return r;
        }
        else
        {
            printf("[ %s ] %s \n",name,buff);
        }

    }
    return r;
}

char *get_name(char name[])
{
    char *s = strtok(name," ");
    if(s != NULL){
        s = strtok(NULL," ");
        if(s != NULL)
        {
            // printf("name got in get_name()  [ %s ]\n",s);
            return s;
        }
    }    
    return NULL;
}

int check_status_after_data_read(int status,thread_info *self)
{
    r_type r = CONT;
    if(status == NAME_ADDED)
    {
        printf("Client added name : %s \n",self->name);
        return r;
    }
    else if(status == TERMINATE_MSG )
    {
        printf("[ %s ] Terminate msg from client \n",self->name);
        r = BREAK;
        return r;
    }
    else if( status == REQ_LIST_T )
    {
        //client requested the list
        int n=0;
        char **list = get_list(self->head, &n);
        char *frame = make_frame(list,n);
        send(SELFFD,frame,strlen(frame),0);
        return r;
    }
    printf("Error Occured in data checking [ 0 ] [ %d ]\n",status);
    return BREAK;
}