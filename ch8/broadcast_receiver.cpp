#include "../common.h"

#define LOCAL_PORT 9000
#define BUFFER_SIZE 512

int main()
{
    int ret;

    SOCKET receiver_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(receiver_socket == INVALID_SOCKET) error_quit("socket()");

    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(LOCAL_PORT);
    ret = bind(receiver_socket, (struct sockaddr*)&local_addr, sizeof(local_addr));
    if(ret == SOCKET_ERROR) error_quit("bind()");

    struct sockaddr_in peer_addr;
    socklen_t addr_len;
    char buffer[BUFFER_SIZE + 1];

    while(true)
    {
        addr_len = sizeof(peer_addr);
        ret = recvfrom(receiver_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&peer_addr, &addr_len);
        if(ret == SOCKET_ERROR)
        {
            error_display("recvfrom()");
            break;
        }

        buffer[ret] = '\0';
        char addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &peer_addr.sin_addr, addr, sizeof(addr));
        printf("[UDP/%s:%d] %s\n", addr, ntohs(peer_addr.sin_port), buffer);
    }

    close(receiver_socket);
    return 0;
}