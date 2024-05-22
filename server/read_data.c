/**
 *  @file read_data.c
 *  
 *  @brief this file contains the functions related to reading the data from the fd if there is anything to read,and parse the
 *         data after reading means if user pressed some cmd and want to covey something to server, so figure out it, and take decision
 *         accordingly and return to the calling function back with apropriate return value, which we can utilize for furter actions
 *         in calling function.
 * 
 *  @author Jigar H. Maheshwari
 *  @date May 22 2024
 * 
*/

#include "../header/server.h"

extern char temp_str[];

/**
 *
 *  @brief check_for_data(); function called each time any data is available to be read on client's fd, this function will parse the string and check what client wants to communicate
 *         with us, if it requests user's list or want to add the name or wants to connect with other client check for it and do actions accordingly.
 * 
 *  @param [thread info *self]
 *         this is the pointer of the info. struct for which we want to read the data and take the decision, we need fd and name so need this info structure here
 *  @param [char **buff]
 *         this is the buffer in which we want to store the data which we read from the client's fd 
 * 
 *  @return [int]
 *          SUCCESS is everything goes fine, other wise errorcode or other information code related to the cmd entered by the user.
 * 
*/
int check_for_data(thread_info *self, char *buff)
{
    struct pollfd fd = self->fds[0];                                    /*!< in self structure 0th fd in pollfd is for client's fd which we need to consider here . */

    r_type r = SUCCESS;
    if (fd.revents & POLLIN)                                            /** if there is anything to be read from client's fd than go further  */
    {
        memset(buff, '\0', BUFSIZE);                                    /** reset the recieve buffer with nullls  */
        int bytes_read = recv(fd.fd, buff, BUFSIZE, 0);                 /** read data from client's fd and store return value in bytes read for further use */
        if (!bytes_read)                                                /** is bytes read == 0, it means something went wrong on other side */
        {
            RST_LOG_BFR;
            sprintf(logMsg, "[ %s ] NULL Bytes Received",self->name );
            ADDLOG;
            
            r = NULL_BYTES;                                                     /** return from the function with return value as a NULL_BYTES for further action in calling function  */
            return r;
        }
        else if (self->busy_flag == 0)                                          /** we go further in this branch if we are free now, means not in active chat with anyone else */  
        {
            if (!strcmp(buff, TERMINATE))                                       /** if the client has sent the terminate message than just add one log into log file about that and return with TERMINATE_MSG value */
            {
                RST_LOG_BFR;
                sprintf(logMsg, "[ %s ] Terminate msg received",self->name );
                ADDLOG;

                // printf("Terminate msg fron client [ %s ] \n",name);
                r = TERMINATE_MSG;
                return r;
            }
            else if (!(strncmp(buff, REPLNAME, strlen(REPLNAME))))              /** if the message from client start with REPL_NAME it means client has sent its name, and we have to fill that name into self information structure for further communication with other clients */
            {
                char *name_temp = get_name(buff);                               /** fetch the name from the clients' message, means remove REPL_NAME ahead of name*/
                if (name_temp == NULL)
                {
                    r = NULL_PTR;
                    return r;
                }

                int s  = check_name(self, name_temp);                           /** check for the name, if other client exist with same name than current client is not allowed to add this name and asked for the new name */
                if(s == FOUND )
                {
                    printf("Name already exits \n");
                    send(SELFFD, NAME_EXIST, strlen(NAME_EXIST), 0);            /** send message to client that this name has taken, give any else name */
                    usleep(100);
                    //request new name again
                    send(SELFFD, REQ_NAME, strlen(REQ_NAME), 0);                /** send request for new name */

                    RST_LOG_BFR;
                    sprintf(logMsg, "[ %s ] Name Already Exist",name_temp );
                    ADDLOG;

                    return FOUND;
                }
                else if(s == NULL_PTR )                                         /** if no member's are present in the list at that time we return from here with return value as NULL_LIST*/
                {
                    printf("NULL PTR FOUND \n");
                    return NULL_LIST;
                }

                printf("[ %s ] Name added \n", name_temp);                      /** if this new name is not taken already than add it as a name of current client */
                // memset(name,'\0',strlen(UNDEF));
                strcpy(self->name, name_temp);
                
                RST_LOG_BFR;
                sprintf(logMsg, "[ %s ] Name Added",name_temp );
                ADDLOG;

                r = NAME_ADDED;
                return r;
            }
            else if (!strcmp(buff, REQ_LIST))                                   /** if current client requests the list of available users than go further in this branch */
            {
                printf("Client requested a list of users \n");
                RST_LOG_BFR;
                sprintf(logMsg, "[ %s ] Requested Client list",self->name );
                ADDLOG;
                r = REQ_LIST_T;                                                 /** return with value REQ_LIST_T and take apropriate actions in calling function */
                return r;
            }
            else if (!strncmp(buff, CONN_REQ, strlen(CONN_REQ)))                /** if current client wants to make connection with other client than go further in this branch */
            {
                char *name_temp_2 = get_name(buff);                             /** fetch the name out from the connection request message */
                if (name_temp_2 == NULL)
                {
                    r = NULL_PTR;
                }
                else if( !strcmp(name_temp_2, UNDEF) )                          /** if name is same as UNDEF then send client error message and wait for other command from client*/
                {
                    printf("[ %s ] trying to connect unknown \n", self->name);
                    send(SELFFD, UNDEF_CONN, strlen(UNDEF_CONN), 0);
                    RST_LOG_BFR;
                    sprintf(logMsg, "[ %s ] Trying to connect client with no name",self->name );
                    ADDLOG;
                    r = CONN_UND;
                }
                else if (!strcmp(self->name, name_temp_2))                      /** if the connection name and client's own name is same, than send error message to client and wait for other command from the client*/
                {
                    printf("[ %s ] trying to connect with it self \n", self->name);
                    send(SELFFD, SAME_USER, strlen(SAME_USER), 0);
                    RST_LOG_BFR;
                    sprintf(logMsg, "[ %s ] Trying to connect with itself",self->name );
                    ADDLOG;
                    r = CONN_WITH_ITSELF;
                }
                else                                                            /** if we came till here means everything with connection nmae is completely fine and check further for busyness of connection */
                {   
                    printf("[ %s --> %s ] connection request \n", self->name, name_temp_2);    
                    RST_LOG_BFR;
                    sprintf(logMsg, "[ %s --> %s ] connecting request",self->name ,name_temp_2);
                    ADDLOG;

                    pthread_mutex_lock(&listMutex);
                    int s = check_and_connect(self, name_temp_2);               /** this function check_and_connect will check weathre requested client is free or not and if it is free than it will make connection with it */
                    pthread_mutex_unlock(&listMutex);
                    if (s == BUSY)
                    {
                        printf("[ %s ] is busy \n", name_temp_2);
                        sprintf(temp_str, "%s is busy right now", name_temp_2);
                        send(SELFFD, temp_str, strlen(temp_str), 0);
                        RST_LOG_BFR;
                        sprintf(logMsg, "[ %s ] is busy right now", name_temp_2 );
                        ADDLOG;
                    }
                    else if (s == USER_NOT_FOUND)                               /** if there is no user with the name provided by the client than send message to client informing that and wait for other command from the client */
                    {
                        printf("User not found with name [ %s ]\n", name_temp_2);
                        sprintf(temp_str, "user not found with name [ %s ]", name_temp_2);
                        send(SELFFD, temp_str, strlen(temp_str), 0);
                        RST_LOG_BFR;
                        sprintf(logMsg, "[ %s ] User not found", name_temp_2 );
                        ADDLOG;
                    }
                    else if (s == SUCCESS)                                      /** if check and connect return SUCCESS it means we have successfully connected with other clietn and can chat with him from now */
                    {
                        printf("[ %s --> %s ] connection established \n", self->name, name_temp_2);
                        sprintf(temp_str, "connection succesfull with [ %s ]", name_temp_2);
                        send(SELFFD, temp_str, strlen(temp_str), 0);
                        sprintf(temp_str, "connection succesfull with [ %s ]", self->name);
                        send(CONNFD, temp_str, strlen(temp_str), 0);
                        RST_LOG_BFR;
                        sprintf(logMsg, "[  %s --> %s ] connection established", self->name, name_temp_2 );
                        ADDLOG;
                    }
                    r = CONN_REQ_T;
                }
                return r;
            }
            else                                                                /** if nothing from above came to server it is considered as a INVALID_CMD and its message is sent to client and waits for other command from client */
            {
                printf("Invalid CMD \n");
                send(SELFFD, ILLG_CMD, strlen(ILLG_CMD), 0);
                RST_LOG_BFR;
                sprintf(logMsg, "[ %s ] sent invalid command", self->name );
                ADDLOG;
                return SUCCESS;
            }
        }
        else if (self->busy_flag == 1)                                                              /** the below branch is executed only when we are connected with some other client and busy flag is BUSY */
        {
            if (!strcmp(buff, TERMINATE))                                                           /** if we are in active chat and send message == EXIT pr pressed ctrl+C on client side than we need to send ack. about our termination to other side client*/
            {
                printf("Terminate msg fron client [ %s ] while busy \n", self->name);
                send(CONNFD, CHAT_OVER, strlen(CHAT_OVER), 0);                                      /** send BYE message to other side client in this case */
                printf("chat over with [ %s ---> %s ] \n", self->name, self->conn_name);

                RST_LOG_BFR;
                sprintf(logMsg, "[ %s ] Terminate msg fron client while busy", self->name );
                ADDLOG;
                usleep(1000);
                sprintf(temp_str, "[ %s ] disconnected with you...", self->name);
                send(CONNFD, temp_str, strlen(temp_str), 0);
                RST_LOG_BFR;
                sprintf(logMsg, "[ %s ---> %s ] chat over", self->name, self->conn_name );
                ADDLOG;
                pthread_mutex_lock(&listMutex);
                del_connection(self);                                                               /** here in this function we will reset noth client's busy flag to 0, means they can connect with others from now */                                       
                pthread_mutex_unlock(&listMutex);
                r = TERMINATE_MSG_BUSY;
                return r;
            }
            else if( !strcmp(buff, CHAT_OVER) ) 
            {
                //we want to quit
                printf("Bye msg fron client [ %s ] \n", self->name);
                sprintf(temp_str, "[ %s ] disconnected with you...", self->name);
                send(CONNFD, temp_str, strlen(temp_str), 0);
                
                RST_LOG_BFR;
                sprintf(logMsg, "[ %s ---> %s ] disconnected with bye msg ...", self->name, self->conn_name );
                ADDLOG;

                usleep(10000);

                send(CONNFD, CHAT_OVER, strlen(CHAT_OVER), 0);
                printf("chat over with [ %s ---> %s ] \n", self->name, self->conn_name);

                RST_LOG_BFR;
                sprintf(logMsg, "[ %s ---> %s ] chat over with", self->name, self->conn_name );
                ADDLOG;

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

int check_name(thread_info *self,char *name)
{
    r_type r = USER_NOT_FOUND;
    int n=0;
    char **list = get_list((self->head),&n);
    if( list == NULL )
    {
        return NULL_PTR;
    }
    for(int i = 0; i< n; i++ )
    {
        printf("[ %s == %s ]\n",name,list[i]);
        if( !strcmp(name,list[i]))
        {
            printf("Found name already exists \n");
            r = FOUND;
            break;
        }
    }
    return r;
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
            free(list);
        }
        printf("status [ %s ][ %d ]\n",self->name,status);
        r = CONT;
    }
    else if ( (status == CONN_REQ_T) || (status == CONN_WITH_ITSELF) || (status == BYE_T) || (status == TERMINATE_MSG_BUSY) || (status == CONN_UND) || (status == FOUND))
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