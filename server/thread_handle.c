#include "../header/server.h"

void *thread_handle(void *arg)
{
    thread_info *self = (thread_info *)arg;
    self->tid = pthread_self();
    int bytes_read = 0;
    char buffer[BUFSIZE]; // rec buffer
    char msg[BUFSIZE];    // send buffer

    memset(buffer, '\0', sizeof(buffer));
    memset(msg, '\0', sizeof(msg));

    strcpy(msg, REQ_NAME);

    send(SELFFD, REQ_NAME, strlen(REQ_NAME), 0);

    while (self->run_flag)
    {
        int status = poll(self->fds, 2, T_TIMEOUT);
        if (status == -1)
        {
            // printf("Poll Error..\n");
            break;
        }
        else if (self->fds[0].revents & POLLIN)
        {
            // something came from client
            memset(msg, '\0', strlen(msg));
            bytes_read = recv(SELFFD, msg, BUFSIZE - 1, 0);
            if (!(bytes_read))
                break;

            msg[bytes_read] = '\0'; // to terminate properly
            if (!(strncmp(msg, TERMINATE, strlen(TERMINATE))))
            {
                printf("Termination signal from client [ %s ]...\n",self->name);
                if(self->busy_flag)
                {
                    pthread_mutex_lock(&listMutex);
                    self->busy_flag = 0;
                    pthread_mutex_unlock(&listMutex);
                    printf("[ %s ]Disconnected \n", self->conn_name);
                    printf("here\n");
                    send(CONNFD,CHAT_OVER,strlen(CHAT_OVER),0);
                }
                break;
            }
            else if( !(self->busy_flag) )
            {
                if ( !(strncmp(msg, REPLNAME, strlen(REPLNAME))) )
                {
                    char *s = strtok(msg, " ");
                    s = strtok(NULL, " ");
                    strcpy(self->name, s);
                    printf("Name Set : %s \n", self->name);
                }
                else if( !(strncmp(msg, REQ_LIST, strlen(REQ_LIST))) )
                {
                    //client has requested the list of available users
                    printf("Client requested a list of clients...\n");
                    int n;
                    pthread_mutex_lock(&listMutex);
                    char **list = get_list(self->head,&n);
                    pthread_mutex_unlock(&listMutex);
                    char *frame = make_frame(list,n);
                    send(SELFFD,frame,strlen(frame),0);
                    continue;
                }
                else if( !(strncmp(msg, CONN_REQ, strlen(CONN_REQ))) )
                {
                    //client has requested the list of available users
                    printf("Client wants to make connection...\n");

                    char *s = strtok(msg,":");
                    if(s != NULL)
                        s = strtok(NULL," ");
                    if( !strcmp(s,self->name) )
                    {
                        printf("Connection with yourself is NOT Allowed...\n");
                        send(SELFFD,SAME_USER,strlen(SAME_USER),0);
                        continue;
                    }
                    printf("[ %s wants to make connection with %s ]\n",self->name, s );
                    printf("Checking %s is free or not...\n",s);
                    pthread_mutex_lock(&listMutex);
                    int free = check_and_connect(self, s );
                    pthread_mutex_unlock(&listMutex);
                    if(free != SUCCESS){
                        if(free == NULL_LIST )
                        {
                            printf("No members in the list...\n");
                        }
                        else if(free == BUSY )
                        {
                            printf("[ %s ] is busy ...\n",s);
                        }
                        else if( free == USER_NOT_FOUND )
                        {
                            printf("USER NOT FOUND \n");
                        }
                        continue;   
                    }
                    printf("[ %s SUCCESFULLY CONNECTED WITH %s ] \n",self->name,s);
                    send(SELFFD,CONN_SUCC,strlen(CONN_SUCC),0);
                    char connected_msg[100] = "You Are connected with : ";
                    strcat(connected_msg,s);
                    send(CONNFD,CONN_SUCC,strlen(CONN_SUCC),0);
                    send(CONNFD,connected_msg,strlen(connected_msg),0);
                    continue;
                }
                else
                {
                    printf("[ %s ] : sent illigal command \n", self->name);
                    send(SELFFD,ILLG_CMD,strlen(ILLG_CMD),0);
                }
            }
            else if( (self->busy_flag) )
            {
                //here whatever comes send back to other clients
                if(!strcmp(msg,CHAT_OVER))
                {
                    pthread_mutex_lock(&listMutex);
                    self->busy_flag = 0;
                    pthread_mutex_unlock(&listMutex);
                    printf("[ %s ] sent BYE to [ %s ] \n", self->conn_name,self->name);
                }
                send(CONNFD,msg,strlen(msg),0);
                continue;
            }
        }
        else if(self->fds[1].revents & POLLIN && (self->busy_flag))
        {
            //other side connected client has sent us something.
            memset(buffer, '\0', strlen(buffer));
            bytes_read = recv(CONNFD, buffer, BUFSIZE - 1, 0);
            if (!(bytes_read))
                break;

            buffer[bytes_read] = '\0'; // to terminate properly
            if( !strcmp(buffer,CHAT_OVER))
            {
                printf("[ %s ] is breaking connection with [ %s ]\n",self->name,self->conn_name);
                send(SELFFD,CHAT_OVER,strlen(CHAT_OVER),0);
                pthread_mutex_lock(&listMutex);
                self->busy_flag = 0;
                pthread_mutex_unlock(&listMutex);
                continue;   

            }
            else if( !strcmp(buffer,TERMINATE))
            {
                printf("Termination msgfrom other side cleint [%s ] got from [ %s ] \n",self->name,self->conn_name);
            }
            send(SELFFD,buffer,strlen(buffer),0);
        }
    }
    pthread_mutex_lock(&listMutex);
    if ( (self->busy_flag))
    {
        printf("Sending BYE signal to [ %s ] from [ %s ]...\n",self->name,self->conn_name);
        send(SELFFD, CHAT_OVER, strlen(CHAT_OVER), 0);
    }
    self->run_flag = 0;
    pthread_mutex_unlock(&listMutex);
    usleep(1000);
    close(SELFFD);
    pthread_mutex_lock(&listMutex);
    del_node(self);
    pthread_mutex_unlock(&listMutex);
    printf("[ %s ] over....\n", self->name);
    return NULL;
}
