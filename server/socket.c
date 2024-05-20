#include "../header/server.h"

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

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

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

    *fd = server_fd;
    *addr = address;
    return r;
}

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