#include "../common.h"

#define MULTICAST_IP "235.7.8.9"
#define PORT 9000
#define BUFFER_SIZE 512

pthread_mutex_t mutex;

void* receiver_thread(void* arg)
{
    int ret;

    SOCKET receiver_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(receiver_socket == INVALID_SOCKET) error_quit("socket()");

    int opt = 1;
    ret = setsockopt(receiver_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if(ret == SOCKET_ERROR) error_quit("setsockopt()");

    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(PORT);
    ret = bind(receiver_socket, (struct sockaddr*)&local_addr, sizeof(local_addr));
    if(ret == SOCKET_ERROR) error_quit("bind()");

    struct ip_mreq mreq;
    inet_pton(AF_INET, MULTICAST_IP, &mreq.imr_multiaddr);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    ret = setsockopt(receiver_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
    if(ret == SOCKET_ERROR) error_quit("setsockopt()");

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

    ret = setsockopt(receiver_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
    if(ret == SOCKET_ERROR) error_quit("setsockopt()");

    close(receiver_socket);

    return NULL;
}

void* sender_thread(void* arg)
{
    int ret;

    SOCKET sender_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(sender_socket == INVALID_SOCKET) error_quit("socket()");

    u_char ttl = 2;
    ret = setsockopt(sender_socket, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
    if(ret == SOCKET_ERROR) error_quit("setsockopt()");

    struct sockaddr_in remote_addr;
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    inet_pton(AF_INET, MULTICAST_IP, &remote_addr.sin_addr);
    remote_addr.sin_port = htons(PORT);

    char buffer[BUFFER_SIZE + 1];
    int len;

    while(true)
    {
        if(fgets(buffer, BUFFER_SIZE + 1, stdin) == NULL)
            break;
        
        len = (int)strlen(buffer);
        if(buffer[len - 1] == '\n')
            buffer[len - 1] = '\0';
        if(strlen(buffer) == 0)
            break;
        
        ret = sendto(sender_socket, buffer, (int)strlen(buffer), 0, (struct sockaddr*)&remote_addr, sizeof(remote_addr));
        if(ret == SOCKET_ERROR)
        {
            error_display("sendto()");
            break;
        }
        //printf("[UDP] send %d bytes.\n", ret);
    }    

    close(sender_socket);

    return NULL;
}

int main()
{
    int ret;
    pthread_mutex_init(&mutex, NULL);

    pthread_t tid[2];
    ret = pthread_create(&tid[0], NULL, receiver_thread, NULL);
    if(ret != 0) return 1;
    ret = pthread_create(&tid[1], NULL, sender_thread, NULL);
    if(ret != 0) return 1;

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);

    pthread_mutex_destroy(&mutex);

    return 0;
}