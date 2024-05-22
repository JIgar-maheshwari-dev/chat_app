#include "../header/server.h"


/**
 * temp_str buffer for storing temporary messages that we want to send to someone
*/
char temp_str[BUFSIZE + 100];


/**
 *  thread handle function which will be for each thread created
*/
void *thread_handle(void *arg)
{
    thread_info *self = (thread_info *)arg;      /*!< thread_info struct pointer for current client's information access, like fd, name etc. */

    self->tid = pthread_self();
    char buffer[BUFSIZE];                       /*!< buffer for receiving data . */
    char msg[BUFSIZE];                          /*!< buffer for sending data . */

    memset(buffer, '\0', BUFSIZE );
    memset(msg, '\0', BUFSIZE );

    strcpy(msg, REQ_NAME);

    /**
     * by default when new thread is created at that time send request to client for sending the name of it 
    */
    send(SELFFD, REQ_NAME, strlen(REQ_NAME), 0);
    /**
     * loop till out thread's run_flag is not zero
    */
    while (self->run_flag)
    {
        /**
         * poll current client's fd, check for any data to be read or not, if there is any data to be read then read and process accordingly
         * 
        */
        int status = poll(self->fds, 1, T_TIMEOUT);
        if (status == -1)
        {
            // printf("Poll Error..\n");
            break;
        }
        /**
         *  if any data is available on our fd than check what message we have recieved and do actions accordingly 
         **/
        status = check_for_data(self, msg); // checking data from self client side
        if (status != SUCCESS)
        {
            /**
             * after checking the data we need to take decisoin weather we have to continue the loop or have to break, if such cmd is recieved than do action accordingly
            */
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

    /**
     * after coming out of loop, we need to check by which reason we came out of loop, if the reason is being run_flag = 0, means server turned off, so we have to send 
     * termination message to all the clients connected with us, so they can also terminate gracefuly.
     * 
    */
    if (!(self->run_flag)) // send termination signal to client if we initiated termination
    {
        printf("Sending Terminate msg to [ %s ] \n", self->name);
        send(SELFFD, TERMINATE, strlen(TERMINATE), 0);
        usleep(1000);
    }
    close(SELFFD);

    RST_LOG_BFR;
    sprintf(logMsg, "[ %s ] Client disconnected...",self->name );
    ADDLOG;

    pthread_mutex_lock(&listMutex);
    /**
     *  if we are reached till this point means client don't more need any kind of connection with the server and don;t want to use
     *  chatting facility so delete the node related to current client from the linked list and free that up memory for other use.
     * 
    */
    del_node(self);
    pthread_mutex_unlock(&listMutex);
    printf("[ %s ] over....\n", self->name);
    return NULL;
}

