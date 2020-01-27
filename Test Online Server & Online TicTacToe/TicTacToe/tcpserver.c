#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
short SocketCreate(void)
{
    short hSocket;
    printf("Create the socket\n");
    hSocket = socket(AF_INET, SOCK_STREAM, 0);
    return hSocket;
}
int BindCreatedSocket(int hSocket, int ClientPort)
{
    int iRetval=-1;
    // int ClientPort = 45678;
    struct sockaddr_in  remote= {0};
    /* Internet address family */
    remote.sin_family = AF_INET;
    /* Any incoming interface */
    remote.sin_addr.s_addr = htonl(INADDR_ANY);
    remote.sin_port = htons(ClientPort); /* Local port */
    iRetval = bind(hSocket,(struct sockaddr *)&remote,sizeof(remote));
    return iRetval;
}
int main(int argc, char *argv[])
{
    int socket_desc, sock, sock1, clientLen, read_size;
    struct sockaddr_in server, client;
    char client_message[50]= {0};
    char message[50] = {0};
    const char *pMessage = "hello aticleworld.com";
    //Create socket
    socket_desc = SocketCreate();
    if (socket_desc == -1)
    {
        printf("Could not create socket");
        return 1;
    }
    printf("Port num %d\n", atoi(argv[1]));
    printf("Socket created\n");
    //Bind
    if( BindCreatedSocket(socket_desc, atoi(argv[1])) < 0)
    {
        //print the error message
        perror("bind failed.");
        return 1;
    }
    printf("bind done\n");
    //Listen
    listen(socket_desc, 3);
    //Accept and incoming connection
    // while(1)
    // {
restart:
        printf("Waiting for incoming connections...\n");
        clientLen = sizeof(struct sockaddr_in);
        //accept connection from an incoming client
        sock = accept(socket_desc,(struct sockaddr *)&client,(socklen_t*)&clientLen);
        if (sock < 0)
        {
            perror("accept failed");
            return 1;
        }
        printf("Player1 Connection accepted\n");

        sock1 = accept(socket_desc,(struct sockaddr *)&client,(socklen_t*)&clientLen);
        if (sock1 < 0)
        {
            perror("accept failed");
            return 1;
        }
        printf("Player2 Connection accepted\n");


        memset(client_message, '\0', sizeof client_message);
        memset(message, '\0', sizeof message);

    while(1) {
        //Receive a reply from the client
        if( recv(sock, client_message, 200, 0) < 0)
        {
            printf("recv failed");
            break;
        }
        printf("Client1 reply : %s\n",client_message);
        // Send some data
        if( send(sock1, client_message, strlen(client_message), 0) < 0)
        {
            printf("Send failed");
            return 1;
        }
        if(strcmp(client_message, "win") == 0)
            break;

        //Receive a reply from the client
        if( recv(sock1, client_message, 200, 0) < 0)
        {
            printf("recv failed");
            break;
        }
        printf("Client2 reply : %s\n",client_message);
        // Send some data
        if( send(sock, client_message, strlen(client_message), 0) < 0)
        {
            printf("Send failed");
            return 1;
        }
        if(strcmp(client_message, "win") == 0)
            break;
        sleep(1);
    }
    goto restart;
    return 0;
}