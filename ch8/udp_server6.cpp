#include "../common.h"

#define SERVER_PORT 9000
#define BUFFER_SIZE 512

int main()
{
    int ret;

    SOCKET server_socket = socket(AF_INET6, SOCK_DGRAM, 0);
    if(server_socket == INVALID_SOCKET) error_quit("socket()");

    struct sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(SERVER_PORT);
    ret = bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret == SOCKET_ERROR) error_quit("bind()");

    struct sockaddr_in6 client_addr;
    socklen_t addr_len;
    char buffer[BUFFER_SIZE + 1];

    while(true)
    {
        addr_len = sizeof(client_addr);
        ret = recvfrom(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &addr_len);
        if(ret == SOCKET_ERROR)
        {
            error_display("recvfrom()");
            break;
        }

        buffer[ret] = '\0';
        char addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin6_addr, addr, sizeof(addr));
        printf("[UDP/%s:%d] %s\n", addr, ntohs(client_addr.sin6_port), buffer);

        ret = sendto(server_socket, buffer, ret, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
        if(ret == SOCKET_ERROR)
        {
            error_display("sendto()");
            break;
        }
    }

    close(server_socket);
    return 0;
}