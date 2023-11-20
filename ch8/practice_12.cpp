#include "../common.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9000
#define BUFFER_SIZE 512

int main()
{
    int ret;

    SOCKET client_socket = socket(AF_INET, SOCK_DGRAM, 0);
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

    struct sockaddr_in local_addr;
    socklen_t addr_len = sizeof(local_addr);         
    ret = getsockname(client_socket, (struct sockaddr*)&local_addr, &addr_len);
    if(ret == SOCKET_ERROR) error_quit("detsockname()");
    char local_addr_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &local_addr.sin_addr, local_addr_str, sizeof(local_addr_str));
    printf("[UDP client] local IP addr =%s Port num =%d", local_addr_str, ntohs(local_addr.sin_port));

    while(true)
    {
        printf("\n[Data] :");
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
            error_display("sendto()");
            break;
        }
        printf("[UDP client] send %d bytes.\n", ret);

        ret = recv(client_socket, buffer, ret, MSG_WAITALL);
        if(ret == SOCKET_ERROR)
        {
            error_display("recvfrom()");
            break;
        }

        buffer[ret] = '\0';
        printf("[UDP client] received %d bytes.\n", ret);
        printf("[UDP client] received data : %s\n", buffer);
        
    }

    close(client_socket);
    return 0;
}