#include "../common.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9000
#define BUFFER_SIZE 512

int main(int argc, char* argv[])
{
    int ret;

    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket == INVALID_SOCKET) error_quit("socket()");

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
    server_addr.sin_port = htons(SERVER_PORT);
    ret = connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret == SOCKET_ERROR) error_quit("connect()");

    char buffer[BUFFER_SIZE + 1];
    int len;


    while(true)
    {
        printf("\n[Data] ");
        if(fgets(buffer, BUFFER_SIZE + 1, stdin) == NULL)
            break;
        
        len = (int)strlen(buffer);
        if(buffer[len - 1] == '\n')
            buffer[len - 1] = '\0';
        if(strlen(buffer) == 0)
            break;
        
        ret = send(client_socket, buffer, (int)strlen(buffer), 0);
        if(ret == SOCKET_ERROR)
        {
            error_display("send()");
            break;
        }
        printf("[TCP client] send %d bytes\n", ret);

        ret = recv(client_socket, buffer, ret, MSG_WAITALL);
        if(ret == SOCKET_ERROR)
        {
            error_display("recv()");
            break;
        }
        else if(ret == 0)
            break;
        
        buffer[ret] = '\0';
        printf("[TCP client] recv %d bytes.\n", ret);
        printf("[TCP client] received data = %s\n", buffer);
    }

    close(client_socket);
    return 0;
}