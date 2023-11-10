#include "../common.h"

#define SERVER_PORT 9000
#define BUFFER_SIZE 512

void* TCP_server4(void *arg)
{
    int retval;

    SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_socket == INVALID_SOCKET) error_quit("socket()");

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);
    retval = bind(listen_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(retval == SOCKET_ERROR) error_quit("bind()");

    retval = listen(listen_socket, SOMAXCONN);
    if(retval == SOCKET_ERROR) error_quit("listen()");

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

        printf("\n[TCP server] client accept : IP addr=%s, Port Num=%d\n", 
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        while(true)
        {
            retval = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if(retval == SOCKET_ERROR)
            {
                error_display("recv()");
                break;
            }
            else if(retval == 0)
                break;

            buffer[retval] = '\0';
            printf("%s", buffer);
        }

        close(client_socket);
        printf("\n[TCP server] client close : IP addr=%s, Port Num=%d\n",
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }

    close(listen_socket);
    return 0;
}

void* TCP_server6(void *arg)
{
    int retval;

    SOCKET listen_socket = socket(AF_INET6, SOCK_STREAM, 0);
    if(listen_socket == INVALID_SOCKET) error_quit("socket()");

    int no = 1;
    setsockopt(listen_socket, IPPROTO_IPV6, IPV6_V6ONLY, &no, sizeof(no));

    struct sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(SERVER_PORT);
    retval = bind(listen_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(retval == SOCKET_ERROR) error_quit("bind()");

    retval = listen(listen_socket, SOMAXCONN);
    if(retval == SOCKET_ERROR) error_quit("listen()");

    SOCKET client_socket;
    struct sockaddr_in6 client_addr;
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

        char ip_addr[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &client_addr.sin6_addr, ip_addr, sizeof(ip_addr));
        printf("\n[TCP server] client accept : IP addr=%s, Port Num=%d\n", 
            ip_addr, ntohs(client_addr.sin6_port));
        
        while(true)
        {
            retval = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if(retval == SOCKET_ERROR)
            {
                error_display("recv()");
                break;
            }
            else if(retval == 0)
                break;

            buffer[retval] = '\0';
            printf("%s", buffer);
        }

        close(client_socket);
        printf("\n[TCP server] client close : IP addr=%s, Port Num=%d\n",
            ip_addr, ntohs(client_addr.sin6_port));
    }

    close(listen_socket);
    return 0;
}

int main(int argc, char* argv[])
{
    pthread_t h_thread[2];
    pthread_create(&h_thread[0], NULL, TCP_server4, NULL);
    pthread_create(&h_thread[1], NULL, TCP_server6, NULL);
    pthread_join(h_thread[0], NULL);
    pthread_join(h_thread[1], NULL);
    return 0;
}