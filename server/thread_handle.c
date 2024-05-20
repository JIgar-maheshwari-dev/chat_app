#include "../header/server.h"

char temp_str[BUFSIZE + 100];

void *thread_handle(void *arg)
{
    thread_info *self = (thread_info *)arg;

    self->tid = pthread_self();
    char buffer[BUFSIZE]; // rec buffer
    char msg[BUFSIZE];    // send buffer

    memset(buffer, '\0', BUFSIZE );
    memset(msg, '\0', BUFSIZE );

    strcpy(msg, REQ_NAME);

    send(SELFFD, REQ_NAME, strlen(REQ_NAME), 0);
    print_list(*(self->head));
    while (self->run_flag)
    {
        int status = poll(self->fds, 2, T_TIMEOUT);
        if (status == -1)
        {
            // printf("Poll Error..\n");
            break;
        }
        status = check_for_data(self, msg); // checking data from self client side
        if (status != SUCCESS)
        {
            status = check_status_after_data_read(status, self);
            if (BREAK == status)
            {
                break;
            }
            if (CONT == status)
            {
                continue;
            }
        }
    }
    if (!(self->run_flag)) // send termination signal to client if we initiated termination
    {
        printf("Sending Terminate msg to [ %s ] \n", self->name);
        send(SELFFD, TERMINATE, strlen(TERMINATE), 0);
        usleep(1000);
    }
    close(SELFFD);
    pthread_mutex_lock(&listMutex);
    del_node(self);
    pthread_mutex_unlock(&listMutex);
    printf("[ %s ] over....\n", self->name);
    return NULL;
}

int check_for_data(thread_info *self, char *buff)
{
    struct pollfd fd = self->fds[0];

    r_type r = SUCCESS;
    if (fd.revents & POLLIN)
    {
        // something is to be read from sock_fd
        memset(buff, '\0', BUFSIZE);
        int bytes_read = recv(fd.fd, buff, BUFSIZE, 0);
        if (!bytes_read)
        {
            r = NULL_BYTES;
            return r;
        }
        else if (self->busy_flag == 0) // here we only talking with current client only
        {
            if (!strcmp(buff, TERMINATE))
            {
                // printf("Terminate msg fron client [ %s ] \n",name);
                r = TERMINATE_MSG;
                return r;
            }
            else if (!(strncmp(buff, REPLNAME, strlen(REPLNAME))))
            {
                // client has send its name
                char *name_temp = get_name(buff);
                if (name_temp == NULL)
                {
                    r = NULL_PTR;
                    return r;
                }
                printf("[ %s ] Name added \n", name_temp);
                // memset(name,'\0',strlen(UNDEF));
                strcpy(self->name, name_temp);
                r = NAME_ADDED;
                return r;
            }
            else if (!strcmp(buff, REQ_LIST))
            {
                // client requested the list
                printf("Client requested a list of users \n");
                r = REQ_LIST_T;
                return r;
            }
            else if (!strncmp(buff, CONN_REQ, strlen(CONN_REQ)))
            {
                // client wants to connect with other client
                char *name_temp_2 = get_name(buff);
                if (name_temp_2 == NULL)
                {
                    r = NULL_PTR;
                }
                else if (!strcmp(self->name, name_temp_2))
                {
                    printf("[ %s ] trying to connect with it self \n", self->name);
                    send(SELFFD, SAME_USER, strlen(SAME_USER), 0);
                    r = CONN_WITH_ITSELF;
                }
                else
                {
                    printf("[ %s --> %s ] connection request \n", self->name, name_temp_2);
                    pthread_mutex_lock(&listMutex);
                    int s = check_and_connect(self, name_temp_2);
                    pthread_mutex_unlock(&listMutex);
                    if (s == BUSY)
                    {
                        printf("[ %s ] is busy \n", name_temp_2);
                        sprintf(temp_str, "%s is busy right now \n", name_temp_2);
                        send(SELFFD, temp_str, strlen(temp_str), 0);
                    }
                    else if (s == USER_NOT_FOUND)
                    {
                        printf("User not found with name [ %s ]\n", name_temp_2);
                        sprintf(temp_str, "user not found with name [ %s ]", name_temp_2);
                        send(SELFFD, temp_str, strlen(temp_str), 0);
                    }
                    else if (s == SUCCESS)
                    {

                        printf("[ %s --> %s ] connection established \n", self->name, name_temp_2);
                        sprintf(temp_str, "connection succesfull with [ %s ]", name_temp_2);
                        send(SELFFD, temp_str, strlen(temp_str), 0);
                        sprintf(temp_str, "connection succesfull with [ %s ]", self->name);
                        send(CONNFD, temp_str, strlen(temp_str), 0);
                    }
                    r = CONN_REQ_T;
                }
                return r;
            }
            else
            {
                printf("Invalid CMD \n");
                send(SELFFD, ILLG_CMD, strlen(ILLG_CMD), 0);
                return SUCCESS;
            }
        }
        else if (self->busy_flag == 1) // busy flag is 1 means we are connected with some another client
        {                              // so whatever comes here send back to another side client
            printf("here we are ...\n");
            if (!strcmp(buff, TERMINATE))
            {
                printf("Terminate msg fron client [ %s ] while busy \n", self->name);
                sprintf(temp_str, "[ %s ] disconnected with you...", self->name);
                send(CONNFD, temp_str, strlen(temp_str), 0);
                usleep(10000);

                send(CONNFD, CHAT_OVER, strlen(CHAT_OVER), 0);
                printf("chat over with [ %s ---> %s ] \n", self->name, self->conn_name);
                pthread_mutex_lock(&listMutex);
                del_connection(self);
                pthread_mutex_unlock(&listMutex);
                r = TERMINATE_MSG_BUSY;
                return r;
            }
            else if( !strcmp(buff, CHAT_OVER) ) 
            {
                //we want to quit
                printf("Terminate msg fron client [ %s ] while busy \n", self->name);
                sprintf(temp_str, "[ %s ] disconnected with you...", self->name);
                send(CONNFD, temp_str, strlen(temp_str), 0);
                usleep(10000);

                send(CONNFD, CHAT_OVER, strlen(CHAT_OVER), 0);
                printf("chat over with [ %s ---> %s ] \n", self->name, self->conn_name);
                pthread_mutex_lock(&listMutex);
                del_connection(self);
                pthread_mutex_unlock(&listMutex);
                r = TERMINATE_MSG_BUSY;
                return r;
            }
            else
            {
                printf("[ %s ] sends msg to [ %s ] [ %s ]\n", self->name, self->conn_name, buff);
                send(CONNFD, buff, strlen(buff), 0);
            }
        }
        else
        {
            printf("[ %s ] [ %s ]\n", self->name, buff );
        }
    }
    return r;
}

char *get_name(char name[])
{
    char *s = strtok(name, " ");
    if (s != NULL)
    {
        s = strtok(NULL, " ");
        if (s != NULL)
        {
            // printf("name got in get_name()  [ %s ]\n",s);
            return s;
        }
    }
    return NULL;
}

int check_status_after_data_read(int status, thread_info *self)
{
    r_type r = BREAK;
    if (status == NAME_ADDED)
    {
        printf("Client added name : %s \n", self->name);
        printf("status [ %s ][ %d ]\n",self->name,status);
        r = CONT;
    }
    else if (status == TERMINATE_MSG)
    {
        printf("status [ %s ][ %d ]\n",self->name,status);
        printf("[ %s ] Terminate msg from client \n", self->name);
    }
    else if (status == REQ_LIST_T)
    {
        // client requested the list
        int n = 0;
        char **list = get_list(self->head, &n);
        if (list == NULL)
        {
            printf("NO members in the list \n");
        }
        else
        {
            char *frame = make_frame(list, n);
            send(SELFFD, frame, strlen(frame), 0);
        }
        printf("status [ %s ][ %d ]\n",self->name,status);
        r = CONT;
    }
    else if ( (status == CONN_REQ_T) || (status == CONN_WITH_ITSELF) || (status == BYE_T) || (status == TERMINATE_MSG_BUSY) )
    {
        // client wants to connect with some one
        printf("status [ %s ][ %d ]\n",self->name,status);
        r = CONT;
    }
    else
    {
        printf("Error Occured in data checking [ %s ] [ %d ]\n", self->name,status);
    }
    return r;
}