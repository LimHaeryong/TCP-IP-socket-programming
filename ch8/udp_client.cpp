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

    struct sockaddr_in peer_addr;
    socklen_t addr_len;
    char buffer[BUFFER_SIZE + 1];
    int len;

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
        
        ret = sendto(client_socket, buffer, (int)strlen(buffer), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if(ret == SOCKET_ERROR)
        {
            error_display("sendto()");
            break;
        }
        printf("[UDP client] send %d bytes.\n", ret);

        addr_len = sizeof(peer_addr);
        ret = recvfrom(client_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&peer_addr, &addr_len);
        if(ret == SOCKET_ERROR)
        {
            error_display("recvfrom()");
            break;
        }

        if(memcmp(&peer_addr, &server_addr, sizeof(peer_addr)))
        {
            printf("[Error] wrong data.\n");
            break;
        }

        buffer[ret] = '\0';
        printf("[UDP client] received %d bytes.\n", ret);
        printf("[UDP client] received data : %s\n", buffer);
    }

    close(client_socket);
    return 0;
}