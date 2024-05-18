#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>

#define PORT 8080
#define IP "127.0.0.1"

int cont = 1;

void sig_handler(int sig)
{
    if(sig == SIGINT) cont = 0;
    return;
}

int main(int argc, char const* argv[])
{
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    char msg[1024] = "";
    char buffer[1024] = { 0 };
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        printf("\n Socket creation error \n");
        return -1;
    }
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
 
    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, IP, &serv_addr.sin_addr) <= 0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
 
    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr,sizeof(serv_addr))) < 0) 
    {
        printf("\nConnection Failed \n");
        return -1;
    }


    struct pollfd pfds[2];
    pfds[0].fd = STDIN_FILENO;
    pfds[0].events = POLLIN;
    pfds[1].fd = client_fd;
    pfds[1].events = POLLIN;

    while( (cont) && (strcmp(buffer,"end")) && (poll(pfds, 2, 1000) != -1) )
    {
        memset(msg,'\0',sizeof(msg));
        memset(buffer,'\0',sizeof(buffer));


        if(pfds[0].revents & POLLIN) {
            // read data from stdin and send it over the socket
            read(STDIN_FILENO, msg, 1024);
            send(client_fd, msg, strlen(msg), 0); // repeat as necessary.
        }

        if(pfds[1].revents & POLLIN) {
            // chat data received
            int val = recv(client_fd, buffer, 1024-1,0);
            if(!val) break;
            printf("\n%s", buffer );
        }

        if(pfds[1].revents & (POLLERR | POLLHUP)) {
            // socket was closed
            cont = 0;
        }
    }

    // while(strcmp(msg,"end"))
    // {
    //     memset(msg,'\0',sizeof(msg));
    //     memset(buffer,'\0',sizeof(buffer));
    //     scanf("%[^\n]s",msg);
    //     getchar();
    //     send(client_fd, msg, strlen(msg), 0);
    //     while( ( read(client_fd, buffer,1024 - 1) ) == 0 );     // subtract 1 for the null terminator at the end
    //     printf("%s\n", buffer);
    // }
    // closing the connected socket
    close(client_fd);
    return 0;
}

