#include "../header/server.h"

#define REUSE

static inline void check_n_die(int status)
{
    if (status != SUCCESS)
    {
        printf("FAILURE [ %d ]\n", status);
        exit(EXIT_FAILURE);
    }
}

FILE *fp = NULL;
char logMsg[BUFSIZE + 100] = "";

short run_flag = 1;
short open = 1;

pthread_mutex_t listMutex = PTHREAD_MUTEX_INITIALIZER;

int main()
{
    signal(SIGINT, sig_handler);
    printf("Server started...\n");
    thread_manage *head = NULL;
    fp = fopen("LogFile.log", "a");
    if (fp == NULL)
    {
        open = 0;
    }
    addLog(fp, "SERVER STARTED");
    int server_fd = 0, new_socket = 0;
    struct sockaddr_in address;

    int status = get_socket_s(&server_fd, &address);
    check_n_die(status);
    socklen_t addrlen = sizeof(address);

    struct pollfd sfd[1];
    memset(&sfd, 0, sizeof(sfd));
    sfd[0].fd = server_fd;
    sfd[0].events = POLLIN;

    while (run_flag)
    {
        // printf("Inside main while loop...accepting...\n");

        int rc = poll(sfd, 1, TIMEOUT);
        if (rc < 0)
        {
            // perror("POLL failed");
            break;
        }
        if (sfd[0].revents & POLLIN)
        {
            // something readable on server fd, accept it, server_fd is listening
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            RST_LOG_BFR;
            sprintf(logMsg, "Client connected: IP - %s , FD - %d", inet_ntoa(address.sin_addr), new_socket);
            addLog(fp, logMsg);

            pthread_t t;
            thread_manage *manage = (thread_manage *)malloc(sizeof(thread_manage));
            if (manage == NULL)
            {
                perror("DMA FAILED WHILE ALLOCATIG FOR THREAD_MANAGE");
                exit(EXIT_FAILURE);
            }
            manage->self.head = &head; //giving pointer to the head pointer to all the threads
            pthread_mutex_lock(&listMutex);
            create_thread_manage(manage, new_socket, &head);
            pthread_mutex_unlock(&listMutex);

            pthread_create(&t, NULL, thread_handle, (void *)&(manage->self));
            // printf("[ %ld ]TID \n", t);
        }
    }

    printf("came out of main while loop, run flag = 0\n");
    pthread_mutex_lock(&listMutex);
    close_all_thread(&head);
    pthread_mutex_unlock(&listMutex);
    join_all(&head);
    
    addLog(fp, "SERVER TURNED OFF");
    fclose(fp);
    close_all_fd(&head);
    close(server_fd);
    printf("Server turned OFF...\n");
    return 0;
}

