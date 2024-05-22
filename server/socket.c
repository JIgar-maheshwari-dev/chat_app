/**
 *  @file socket.c
 *  
 *  @brief this file includes functions and utility related to socket access
 *  here one function is for the getting the socket from the system kernel, binding it
 *  to server and IP, and start listening on it
 * 
 * 
 *  @author Jigar H. Maheshwari
 *  @date May 22 2024
 * 
*/

#include "../header/server.h"

/**
 * 
 *  @brief get_socket();
 *         this functions is used to getting the socket and binding it to port and IP, and start listening
 *  
 *  @param [int *fd ] 
 *         we will fill this pointer with new fd we get
 *  
 *  @param [sockaddr_in *addr]
 *         we need this stucture while accept so fill this up with required information
 *  
 *  @return [int ]
 *          SUCCESS in case where everything goes fine, or ERRORCODE in other situatilns
 *  
*/
int get_socket_s(int *fd, struct sockaddr_in *addr)
{
    r_type r = SUCCESS;

    if ((fd == NULL) || (addr == NULL))
    {
        perror("ERR : NULL PTR");
        r = NULL_PTR;
        return r;
    }

    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        r = SOCK_FAIL;
        return r;
    }

    /**
     * if we have used REUSE macro than this code will get executed while creating socket, which
     * is useful in situations where we face bind failure error.
     * 
    */

#ifdef REUSE
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
#endif

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        r = BIND_FAIL;
        return r;
    }

    if (listen(server_fd, BACKLOG) < 0)
    {
        perror("listen");
        r = LISN_FAIL;
        return r;
    }

    /**
     * fill the fd and addr with required information
    */
    *fd = server_fd;
    *addr = address;
    return r;
}


/**
 * 
 * signal handler for SIGINT,to terminate the server gracefully
 * 
*/
void sig_handler(int sig)
{
    pthread_mutex_lock(&listMutex);
    if (sig == SIGINT)
    {
        run_flag = 0;
        printf(" signal came...\n");
    }
    pthread_mutex_unlock(&listMutex);

    return;
}