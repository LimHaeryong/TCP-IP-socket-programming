#include "../common.h"

#define SERVER_PORT 9000
#define BUFFER_SIZE 512

struct SocketInfo
{
    SOCKET sock;
    char buffer[BUFFER_SIZE + 1];
    int recv_bytes;
    int send_bytes;
};

void register_event(int epoll_fd, SOCKET sock, uint32_t events)
{
    SocketInfo *socket_info = new SocketInfo;
    socket_info->sock = sock;
    socket_info->recv_bytes = 0;
    socket_info->send_bytes = 0;

    struct epoll_event ev;
    ev.events = events;
    ev.data.ptr = socket_info;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &ev) < 0)
    {
        perror("epoll_ctl()");
        close(sock);
        exit(1);
    }
}

void modify_event(int epoll_fd, struct epoll_event ev, uint32_t events)
{
    ev.events = events;
    SocketInfo *socket_info = (SocketInfo *)ev.data.ptr;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, socket_info->sock, &ev) < 0)
    {
        perror("epoll_ctl()");
        close(socket_info->sock);
        exit(1);
    }
}

int main()
{
    int ret;

    SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket == INVALID_SOCKET)
        error_quit("socket()");

    int opt = 1;
    ret = setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if(ret == SOCKET_ERROR) error_quit("setsockopt()");

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);
    ret = bind(listen_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret == SOCKET_ERROR)
        error_quit("bind()");

    ret = listen(listen_socket, SOMAXCONN);
    if (ret == SOCKET_ERROR)
        error_quit("listen()");

    int flags = fcntl(listen_socket, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(listen_socket, F_SETFL, flags);

    int epoll_fd = epoll_create(1);
    if (epoll_fd < 0)
    {
        perror("epoll_create()");
        exit(1);
    }

    register_event(epoll_fd, listen_socket, EPOLLIN);
    struct epoll_event events[FD_SETSIZE];

    int num_ready;
    SOCKET client_socket;
    struct sockaddr_in client_addr;
    socklen_t addr_len;

    while (true)
    {
        num_ready = epoll_wait(epoll_fd, events, FD_SETSIZE, -1);
        if (num_ready < 0)
            error_quit("epoll_wait()");

        for (int i = 0; i < num_ready; ++i)
        {
            printf("num ready : %d\n", num_ready);
            SocketInfo *socket_info = (SocketInfo *)events[i].data.ptr;
            if (socket_info->sock == listen_socket)
            {
                printf("listen socket : i = %d\n", i);
                addr_len = sizeof(client_addr);
                client_socket = accept(listen_socket, (struct sockaddr *)&client_addr, &addr_len);
                if (client_socket == INVALID_SOCKET)
                {
                    error_display("accept()");
                    goto MAIN_EXIT;
                }
                else
                {
                    int flags = fcntl(client_socket, F_GETFL);
                    flags |= O_NONBLOCK;
                    fcntl(client_socket, F_SETFL, flags);

                    char addr[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &client_addr.sin_addr, addr, sizeof(addr));
                    printf("\n[TCP server] accept client: IP addr=%s, Port num=%d\n", addr, ntohs(client_addr.sin_port));

                    register_event(epoll_fd, client_socket, EPOLLIN);
                }
            }
            else
            {
                printf("client socket : i = %d\n", i);
                addr_len = sizeof(client_addr);
                getpeername(socket_info->sock, (struct sockaddr *)&client_addr, &addr_len);
                char addr[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &client_addr.sin_addr, addr, sizeof(addr));

                if (events[i].events & EPOLLIN)
                {
                    ret = recv(socket_info->sock, socket_info->buffer, BUFFER_SIZE, 0);
                    if (ret == SOCKET_ERROR)
                    {
                        error_display("recv()");

                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket_info->sock, NULL);
                        close(socket_info->sock);
                        delete socket_info;
                    }
                    else if (ret == 0)
                    {
                        printf("[TCP server] close client : IP addr=%s, Port num=%d\n",
                               addr, ntohs(client_addr.sin_port));

                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket_info->sock, NULL);
                        close(socket_info->sock);
                        delete socket_info;
                    }
                    else
                    {
                        socket_info->recv_bytes = ret;
                        socket_info->buffer[socket_info->recv_bytes] = '\0';
                        printf("[TCP/%s:%d] %s\n", addr,
                               ntohs(client_addr.sin_port), socket_info->buffer);
                        if (socket_info->recv_bytes > socket_info->send_bytes)
                        {
                            modify_event(epoll_fd, events[i], EPOLLOUT);
                        }
                    }
                }
                else if (events[i].events & EPOLLOUT)
                {
                    ret = send(socket_info->sock, socket_info->buffer + socket_info->send_bytes,
                               socket_info->recv_bytes - socket_info->send_bytes, 0);
                    if(ret == SOCKET_ERROR)
                    {
                        error_display("send()");

                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket_info->sock, NULL);
                        close(socket_info->sock);
                        delete socket_info;
                    }
                    else
                    {
                        socket_info->send_bytes += ret;
                        if(socket_info->recv_bytes == socket_info->send_bytes)
                        {
                            socket_info->recv_bytes = socket_info->send_bytes = 0;
                            modify_event(epoll_fd, events[i], EPOLLIN);
                        }
                    }
                }
            }
        }
    }

MAIN_EXIT:

    close(epoll_fd);
    close(listen_socket);

    return 0;
}