#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>

#define PORT 8080

int cont = 1;

void sig_handler(int sig)
{
    if(sig == SIGINT) cont = 0;
    return;
}



int main(int argc, char const* argv[])
{
    int server_fd, new_socket;
    ssize_t valread;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[1024] = { 0 };
    char msg[1024] = "";    
 
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
 
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
 
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address,sizeof(address)) < 0) 
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 3) < 0) 
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr*)&address,&addrlen)) < 0) 
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    struct pollfd pfds[2];
    pfds[0].fd = STDIN_FILENO;
    pfds[0].events = POLLIN;
    pfds[1].fd = new_socket;
    pfds[1].events = POLLIN;

    while( (cont) && (strcmp(buffer,"end")) && (poll(pfds, 2, 1000) != -1) )
    {
        memset(msg,'\0',sizeof(msg));
        memset(buffer,'\0',sizeof(buffer));


        if(pfds[0].revents & POLLIN) {
            // read data from stdin and send it over the socket
            read(STDIN_FILENO, msg, 1024);
            send(new_socket, msg, strlen(msg), 0); // repeat as necessary.
        }

        if(pfds[1].revents & POLLIN) {
            // chat data received
            int val = recv(new_socket, buffer, 1024-1,0);
            if(!val) break;
            printf("\n%s", buffer );
        }

        if(pfds[1].revents & (POLLERR | POLLHUP)) {
            // socket was closed
            cont = 0;
        }
    }
 
    close(new_socket);
    close(server_fd);
    return 0;
}

