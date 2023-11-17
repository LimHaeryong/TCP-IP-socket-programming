#include "../common.h"

#include <string>

#define SERVER_PORT 9000
#define BUFFER_SIZE 512

int main(int argc, char* argv[])
{
    int ret;

    SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_socket == INVALID_SOCKET) error_quit("socket()");

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);
    ret = bind(listen_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret == SOCKET_ERROR) error_quit("bind()");

    ret = listen(listen_socket, SOMAXCONN);
    if(ret == SOCKET_ERROR) error_quit("listen()");

    SOCKET client_socket;
    struct sockaddr_in client_addr;
    socklen_t addr_len;
    char buffer[BUFFER_SIZE + 1];

    while(true)
    {
        addr_len = sizeof(client_addr);
        client_socket = accept(listen_socket, (struct sockaddr*)&client_addr, &addr_len);
        if(client_socket == INVALID_SOCKET)
        {
            error_display("accept()");
            break;
        }

        char addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, addr, sizeof(addr));
        printf("\n[TCP server] client accept : IP addr=%s, port num=%d\n", addr, ntohs(client_addr.sin_port));

        while(true)
        {
            ret = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if(ret == SOCKET_ERROR)
            {
                error_display("recv()");
                break;
            }
            else if(ret == 0)
                break;
            
            buffer[ret] = '\0';
            printf("[TCP/%s:%d] %s\n", addr, ntohs(client_addr.sin_port), buffer);
            for(int i = 0; i < ret; ++i)
            {
                buffer[i] = toupper(buffer[i]);
            }
            ret = send(client_socket, buffer, ret, 0);
            if(ret == SOCKET_ERROR)
            {
                error_display("send()");
                break;
            }
        }

        close(client_socket);
        printf("[TCP server] client close: IP addr=%s, port num=%d\n", addr, ntohs(client_addr.sin_port));
    }

    close(listen_socket);

    return 0;
}