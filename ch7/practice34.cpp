#include "../common.h"

#define SERVER_PORT 9000
#define BUFFER_SIZE 512

struct ProcessClientArgs
{
    SOCKET client_socket;
    struct sockaddr_in client_addr;
};

void* process_client(void* arg)
{
    struct ProcessClientArgs* thread_args = (struct ProcessClientArgs*)(arg);
    int ret;
    SOCKET client_socket = (SOCKET)thread_args->client_socket;
    struct sockaddr_in client_addr = thread_args->client_addr;
    char addr[INET_ADDRSTRLEN];
    char buffer[BUFFER_SIZE + 1];
    inet_ntop(AF_INET, &client_addr.sin_addr, addr, sizeof(addr));

    while(true)
    {
        ret = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if(ret == SOCKET_ERROR)
        {
            error_display("recv()");
            break;
        }
        else if(ret == 0)
        {
            break;
        }

        buffer[ret] = '\0';
        printf("[TCP/%s:%d] %s\n", addr, ntohs(client_addr.sin_port), buffer);

        ret = send(client_socket, buffer, ret, 0);
        if(ret == SOCKET_ERROR)
        {
            error_display("send()");
            break;
        }
    }

    close(client_socket);
    printf("[TCP server] close client: IP addr=%s, Port num=%d\n", addr, ntohs(client_addr.sin_port));

    return 0;
}

int main()
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
    pthread_t tid;

    errno = 0;
    ret = nice(-20);
    if(ret < 0 && errno != 0)
    {
        perror("nice() in test_thread");
        exit(1);
    }

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
        printf("\n[TCP server] client accept: IP addr=%s, Port num=%d\n", addr, ntohs(client_addr.sin_port));

        struct ProcessClientArgs thread_args = {client_socket, client_addr};

        ret = pthread_create(&tid, NULL, process_client, (void*)&thread_args);
        if(ret != 0) close(client_socket);
    }
    close(listen_socket);

    return 0;
}