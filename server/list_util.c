#include "../header/server.h"

void create_thread_manage(thread_manage *temp, int fd, thread_manage **head)
{
    temp->self.tid = 0;
    temp->self.fds[0].fd = fd;         // threads own fd, sock_fd for communication
    temp->self.fds[1].fd = -1;         // here goes connection's fd, default we put -1
    temp->self.fds[0].events = POLLIN; // look for incoming data on own fd
    temp->self.fds[1].events = POLLIN; // look for incoming data on connection's fd
    temp->self.run_flag = 1;
    temp->self.busy_flag = 0;
    memset(temp->self.name, '\0', sizeof(temp->self.name));
    memset(temp->self.conn_name, '\0', sizeof(temp->self.conn_name));
    strcpy(temp->self.name, UNDEF);
    strcpy(temp->self.conn_name, UNDEF);

    temp->next = NULL;
    thread_manage *temp_node = *head;
    if (temp_node == NULL)
    {
        // first node to be added
        printf("Adding HEAD..\n");
        *head = temp;
    }
    else
    {
        while (temp_node->next != NULL)
        {
            temp_node = temp_node->next;
        }
        printf("Adding Node...\n");
        temp_node->next = temp;
    }
}

void close_all_thread(thread_manage **head)
{
    thread_manage *temp = *head;
    if (temp == NULL)
    {
        return;
    }
    printf("Closing All thread, by setting run flag = 0 \n");

    while (temp != NULL)
    {
        if (temp->self.run_flag != 0)
        {
            printf("[ %s ] Setting run flag = 0 \n", temp->self.name);
            temp->self.run_flag = 0;
        }
        temp = temp->next;
    }
    printf("All threads are close...\n");
    return;
}

void join_all(thread_manage **head)
{
    thread_manage *temp = *head;
    if (temp == NULL)
    {
        return;
    }
    printf("Joining All threads...\n");

    while (temp != NULL)
    {
        printf("[ %s ] [ %ld ]Joining....\n", temp->self.name, temp->self.tid);
        if ((pthread_join(temp->self.tid, NULL)) != 0)
        {
            printf("[ %s ] Error Joining Thread \n", temp->self.name);
        }
        temp = temp->next;
    }
    printf("All threads joined...\n");
}

void close_all_fd(thread_manage **head)
{
    thread_manage *temp = *head;
    if (temp != NULL)
    {
        while (temp != NULL)
        {
            close(temp->self.fds[0].fd);
            temp = temp->next;
        }
    }
    return;
}

void addLog(FILE *fp, char *log)
{
    if (open == 1)
    {
        time_t mytime;
        mytime = time(NULL);
        char *time_str = ctime(&mytime);
        time_str[strlen(time_str) - 1] = '\0';
        fprintf(fp, "[ %s ] [ %s ]\n", time_str, log);
    }
}

char **get_list(thread_manage **head, int *number)
{
    int n = 0;
    thread_manage *temp = *head;
    if (temp == NULL)
    {
        printf("No users available to send the list. \n");
        return NULL;
    }
    char **list = (char **)malloc(sizeof(char *) * (++n));

    while (temp != NULL)
    {
        if ( (temp->self.run_flag != 0)   &&   ((strcmp(temp->self.name,UNDEF))) )
        {
            char *name = (char *)malloc(sizeof(char) * (strlen(temp->self.name)));
            name = temp->self.name;
            printf("name %d : %s \n", n, name);
            list[n - 1] = name;
            temp = temp->next;
            if (temp != NULL)
            {
                list = (char **)realloc(list, sizeof(char *) * (++n));
            }
        }
        else
        {
            temp = temp->next;
        }
    }
    *number = n;
    printf("total number of users : %d \n", n);
    return list;
}

char *make_frame(char **list,int n)
{
    char *frame = (char *)malloc(sizeof(char)*BUFSIZE);
    memset(frame,'\0',BUFSIZE);
    strcat(frame,FRAME_START);
    for(int i=0; i<n; i++)
    {
        strcat(frame,MSG_START);
        strcat(frame,list[i]);
    }
    strcat(frame,MSG_START);
    strcat(frame,FRAME_END);

    //printf("[ frame ] [ %s ]\n",frame);
    return frame;
}

void deframe_print(char *frame)
{
        char *s = strtok(frame,DEFRAME);
        while(s != NULL)
        {
                printf("[ %s ]\n",s);
                s=strtok(NULL,DEFRAME);
        }
}

int check_and_connect(thread_info *self, char* name)
{
    r_type r = SUCCESS;
    int flag_name_check = 1;
    thread_manage *temp = *(self->head);
    if( temp == NULL )
    {
        r = NULL_LIST;
        return r;
    }
    while(temp != NULL )
    {
        if( !(strcmp(temp->self.name,name)) )
        {   
            flag_name_check = 0;
            if( !(temp->self.busy_flag) ){
                self->fds[1].fd = temp->self.fds[0].fd;  //self->conn fd = other side client's fd
                temp->self.fds[1].fd = self->fds[0].fd; //other side's conn fd = client's fd
                strcpy(self->conn_name, name);  //conn_name = other side client's name
                strcpy(temp->self.conn_name,self->conn_name); //other side_conn name = self->name
                temp->self.busy_flag = 1;   //set busy flag to 1
                self->busy_flag = 1;    
                return r;
            }
        }
        temp = temp->next;
    }
    if(flag_name_check)
    {
        r = USER_NOT_FOUND;
    }
    else{
        r = BUSY;
    }
    return r;
}




void del_node(thread_info *self)
{

    if( self == NULL )
    {
        return;
    }
    else if(*(self->head) == NULL)
    {
        return;
    }
    else
    {
        thread_manage *temp = *(self->head);
        thread_manage *prev = NULL;
        if(self->fds[0].fd == (*(self->head))->self.fds[0].fd )
        {
            //deleting the head it self
            printf("[ %s ] Node deleted \n",temp->self.name);
            free(temp);
            (*(self->head)) = NULL;
            ((self->head)) = NULL;
            return;
        }
        while( (temp != NULL) && (temp->self.fds[0].fd != SELFFD) )
        {
            prev=temp;
            temp = temp->next;
        }
        if(temp == NULL )
        {
            return;
        }
        else
        {
            printf("[ %s ] Node deleted \n",temp->self.name);
            prev->next = temp->next;
            free(temp);
            temp = NULL;
        }
    }

}