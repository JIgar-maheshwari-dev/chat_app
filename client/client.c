#include "../header/common.h"
short run_flag = 1;

void sig_handler(int sig)
{
    if (sig == SIGINT)
    {
        run_flag = 0;
    }
}

void print_info(void);
void deframe_print(char *frame);

int main()
{
    signal(SIGINT, sig_handler);
    int status, client_fd;
    int bytes_read = 0;
    int read_name = 0;
    int conn_req = 0;
    char buffer[BUFSIZE] = "";
    char msg[BUFSIZE] = "";

    struct sockaddr_in serv_addr;
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

    if ((status = connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
    {
        printf("\nConnection Failed \n");
        return EXIT_FAILURE;
    }

    struct pollfd sfd[2];
    memset(&sfd, 0, sizeof(sfd));
    sfd[0].fd = client_fd;
    sfd[0].events = POLLIN;
    sfd[1].fd = STDIN_FILENO;
    sfd[1].events = POLLIN;

    print_info();

    while (run_flag)
    {
        int status = poll(sfd, 2, TIMEOUT);
        if (-1 == status)
        {
            // perror("POLLING FAILED");
            break;
        }
        else if (sfd[0].revents & POLLIN) // something to recv
        {

            // something available to read from client_fd, means server has sent something to us
            memset(buffer, '\0', BUFSIZE);
            bytes_read = recv(client_fd, buffer, BUFSIZE - 1, 0);
            buffer[bytes_read] = '\0';

            if (!(bytes_read))
            {
                printf("NULL BYTES...\n");
                break;
            }
            else if (!(strncmp(buffer, TERMINATE, strlen(TERMINATE) - 1)))
            {
                printf("Termination signal recieved from server...\n");
                break;
            }
            else if (!(strcmp(buffer, REQ_NAME)))
            {
                write(STDOUT_FILENO, ENTERNAME, strlen(ENTERNAME));
                read_name = 1;
                continue;
            }
            else if( !(strncmp(buffer,DEFRAME,strlen(DEFRAME))))
            {
                deframe_print(buffer);
                continue;
            }

            printf("[ server ] : %s \n", buffer);
        }
        else if (sfd[1].revents & POLLIN) // something to send
        {
            // something available on stdin,read it and send it to the server
            memset(msg, '\0', BUFSIZE);
            int bytes_read = read(STDIN_FILENO, msg, BUFSIZE-1);
            msg[bytes_read-1] = '\0';
            // printf("read from keayboard : %s \n",msg);
            if( read_name )
            {
                char temp_name[20]="";
                strcat(temp_name,REPLNAME);
                strcat(temp_name,msg);
                // printf("Sending name as : %s \n",temp_name);
                send(client_fd, temp_name, strlen(temp_name), 0);
                read_name = 0;
                continue;
            }
            else if( !(strncmp(CONN_REQ,buffer,strlen(buffer))))
            {
                //we want to send the msg of connection request
                conn_req = 1;
            }
            send(client_fd, msg, strlen(msg), 0);
        }
    }
    printf("Out of Loop...\n");

    if (!run_flag)
    {
        printf("Sendig termination msg to server...\n");
        send(client_fd, TERMINATE, strlen(TERMINATE), 0);
    }

    close(client_fd);
    printf("closing app....\n");
    return 0;
}

void print_info(void)
{
    printf("Connected with server successfully \n");
    printf("\"getlist \" To get the list of available users  \n");
    printf("\"connect name \" To get connected with the user with name \n");
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