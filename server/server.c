/**
 * 
 *  @file server.c
 *  
 *  @brief this server file includes main function of server which creates a socket,bind to a port and IP,
 *  and listen for incoming connections with backlog of 10, and then infinitely accepts the incoming connections
 *  until Interrupt is not generated by user to turn off the server.
 * 
 *  @author Jigar H. Maheshwari
 *  @date May 22 2024
 * 
 * 
**/

#include "../header/server.h"

#undef REUSE  ///< Add or remove for reusing the address, for avoiding bind error


/**
 *  @brief check for status value, if status is not success than it will add log into logger file and close the program
 *  useful in socket creation time, to avoid multiple if else and exit statements in main code
*/
static inline void check_n_die(int status)
{
    if (status != SUCCESS)
    {
        printf("FAILURE [ %d ]\n", status);
        addLog(fp, "SERVER TURNED OFF");
        fclose(fp);
        exit(EXIT_FAILURE);
    }
}

FILE *fp = NULL;                    /*!< File for adding logs related to server and connections activity */
char logMsg[BUFSIZE + 100] = "";    /*!< log buffer to store log msg for a while */
short run_flag = 1;                 /*!< run flag to terminate program gracefully, closing all the sockets and after completing all the threads */
short open = 1;                     /*!< open flag for logger file, if there is any error in opening file than our logs are not added to file ,other functionality remains same */

pthread_mutex_t listMutex = PTHREAD_MUTEX_INITIALIZER;      /*!< our linked list is a shared resource, to avoid any kind of race condition and ensure mutual exclusion we need mutex*/


/**
 *  assigns singal handler to SIGINT signal, and creates socket, binds with IP, listen, and accept the connections whenever they come,
 *  and with accepting it will look for run_flag too, if run_flag is 0 then it has to close the program
*/
int main()
{
    signal(SIGINT, sig_handler);
    printf("Server started...\n");

    thread_manage *head = NULL;     /*!< head of our linked list, which holds data of all the active clients */

    /**
     *  opening a logger file, if not opens than set open flag to 0 
    */
    fp = fopen("LogFile.log", "a");
    if (fp == NULL)
    {
        open = 0;
    }

    /**
     * adding first log to our file, that server has started
    */
    addLog(fp, "SERVER STARTED");

    /**
     *  declaration fd variables for both the client and server side 
    */
    int server_fd = 0, new_socket = 0;
    struct sockaddr_in address;


    /**
     * get_socket() function will give us fd, for the server which will listens for incoming connections
     * return the status SUCCESS if everything goes fine
    */
    int status = get_socket_s(&server_fd, &address);
    check_n_die(status);
    socklen_t addrlen = sizeof(address);

    /**
     * declaration of pollfd structure for polling purpose, we will pole server's fd on regular interval
     * and check for run flag too, if any connection is arrived on server's fd than accept it otherwise check run_flag and repreat the loop
     * we are interested if anything is came on server fd or not that's why we have used event as POLLIN with this fd
    */
    struct pollfd sfd[1];
    memset(&sfd, 0, sizeof(sfd));
    sfd[0].fd = server_fd;
    sfd[0].events = POLLIN;

    /**
     * loop till run_flag not becomes Zero
    */
    while (run_flag)
    {
        // printf("Inside main while loop...accepting...\n");
        /**
         * calling poll function, it return either on some event occur on the pollfd sturct provide or timeout has occured
         * if timeout occured it returns 0, it return < 0, values when interrupt occurs,otherwise > 0
        */
        int rc = poll(sfd, 1, TIMEOUT);
        if (rc < 0)
        {
            // perror("POLL failed");
            break;
        }
        /**
         * 
         *  poll will fill the revents by itself when some events occur on fds, we just need to and that values with required events to check
         *  for further actions
         * 
         *  here we are checking that first fd's revents is POLLIN or not, if it is than do thigs inside this if statement,other wise go down
        */
        if (sfd[0].revents & POLLIN)
        {
            /**
             * something readable on server fd, means some connection is arrived to server accept it,
             */
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            ///add log
            RST_LOG_BFR;
            sprintf(logMsg, "Client connected: IP - %s , FD - %d", inet_ntoa(address.sin_addr), new_socket);
            ADDLOG;
            
            /****
             *  allocate a memory for newly came connection's data, add that node into our linked list
             *****/
            pthread_t t;
            thread_manage *manage = (thread_manage *)malloc(sizeof(thread_manage));
            if (manage == NULL)
            {
                perror("DMA FAILED WHILE ALLOCATIG FOR THREAD_MANAGE");
                exit(EXIT_FAILURE);
            }
            manage->self.head = &head; //giving pointer to the head pointer to all the threads
            pthread_mutex_lock(&listMutex);
            /**
             * create new node from the below function, it will default values into client info like name : unknown, fd : -1, busy_flag =0 etc.
             * 
            */
            create_thread_manage(manage, new_socket, &head);
            pthread_mutex_unlock(&listMutex);

            /**
             * create a thread for newly came connection,and pass the pointer to info. structure as an argument to the thread
            */
            pthread_create(&t, NULL, thread_handle, (void *)&(manage->self));
            // printf("[ %ld ]TID \n", t);
        }
    }


    /**
     * if we press ctrl+c we will be here out of above infinite accepting loop
     * here we will close all the connections means set all threds run flag to 0, and then join up them all, and wait for them
     * to complete, then we go for further actions
    */
    printf("came out of main while loop, \n");
    pthread_mutex_lock(&listMutex);
    close_all_thread(&head);
    pthread_mutex_unlock(&listMutex);
    join_all(&head);
    
    /**
     * add last log as server turned off
    */
    addLog(fp, "SERVER TURNED OFF");
    /**close server's fd*
     * 
     */
    fclose(fp);
    close(server_fd);
    printf("Server turned OFF...\n");
    return 0;
}
