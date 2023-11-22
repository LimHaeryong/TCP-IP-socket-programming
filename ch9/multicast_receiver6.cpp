#include "../common.h"

#define MULTICAST_IP "FF12::1:2:3:4"
#define LOCAL_PORT 9000
#define BUFFER_SIZE 512

int main()
{
    int ret;

    SOCKET receiver_socket = socket(AF_INET6, SOCK_DGRAM, 0);
    if(receiver_socket == INVALID_SOCKET) error_quit("socket()");

    int opt = 1;
    ret = setsockopt(receiver_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if(ret == SOCKET_ERROR) error_quit("setsockopt()");

    struct sockaddr_in6 local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin6_family = AF_INET6;
    local_addr.sin6_addr = in6addr_any;
    local_addr.sin6_port = htons(LOCAL_PORT);
    ret = bind(receiver_socket, (struct sockaddr*)&local_addr, sizeof(local_addr));
    if(ret == SOCKET_ERROR) error_quit("bind()");

    struct ipv6_mreq mreq;
    inet_pton(AF_INET6, MULTICAST_IP, &mreq.ipv6mr_multiaddr);
    mreq.ipv6mr_interface = 0;
    ret = setsockopt(receiver_socket, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof(mreq));
    if(ret == SOCKET_ERROR) error_quit("setsockopt()");

    struct sockaddr_in6 peer_addr;
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
        char addr[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &peer_addr.sin6_addr, addr, sizeof(addr));
        printf("[UDP/%s:%d] %s\n", addr, ntohs(peer_addr.sin6_port), buffer);
    }

    ret = setsockopt(receiver_socket, IPPROTO_IPV6, IPV6_LEAVE_GROUP, &mreq, sizeof(mreq));
    if(ret == SOCKET_ERROR) error_quit("setsockopt()");

    close(receiver_socket);

    return 0;
}