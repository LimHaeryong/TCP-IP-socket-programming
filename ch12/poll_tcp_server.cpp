#include "../common.h"

#define SERVER_PORT 9000
#define BUFFER_SIZE 512

struct SocketInfo
{
    char buffer[BUFFER_SIZE + 1];
    int recv_bytes;
    int send_bytes;
};

int num_sockets = 0;
struct SocketInfo* socket_info_arr[FD_SETSIZE];
struct pollfd poll_fd_arr[FD_SETSIZE];

bool add_socket_info(SOCKET sock);
void remove_socket_info(int index);

int main(int argc, char* argv[])
{
    int ret;

    SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_socket == INVALID_SOCKET) error_quit("socket()");

    int opt = 1;
    ret = setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if(ret == SOCKET_ERROR) error_quit("setsockopt()");

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);
    ret = bind(listen_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret == SOCKET_ERROR) error_quit("bind()");

    ret = listen(listen_socket, SOMAXCONN);
    if(ret == SOCKET_ERROR) error_quit("listen()");

    int flags = fcntl(listen_socket, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(listen_socket, F_SETFL, flags);

    poll_fd_arr[0].fd = listen_socket;
    poll_fd_arr[0].events = POLLIN;
    ++num_sockets;

    int num_ready;
    SOCKET client_socket;
    struct sockaddr_in client_addr;
    socklen_t addr_len;

    while(true)
    {
        for(int i = 1; i < num_sockets; ++i)
        {
            if(socket_info_arr[i]->recv_bytes > socket_info_arr[i]->send_bytes)
                poll_fd_arr[i].events = POLLOUT; // write
            else
                poll_fd_arr[i].events = POLLIN; // read
        }

        num_ready = poll(poll_fd_arr, num_sockets, -1);
        if(num_ready == SOCKET_ERROR) error_quit("poll()");

        if(poll_fd_arr[0].revents & POLLIN)
        {
            addr_len = sizeof(client_addr);
            client_socket = accept(listen_socket, (struct sockaddr*)&client_addr, &addr_len);
            if(client_socket == INVALID_SOCKET)
            {
                error_display("accept()");
                break;
            }
            else
            {
                int flags = fcntl(client_socket, F_GETFL);
                flags |= O_NONBLOCK;
                fcntl(client_socket, F_SETFL, flags);

                char addr[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &client_addr.sin_addr, addr, sizeof(addr));
                printf("\n[TCP server] accept client: IP addr=%s, Port num=%d\n", addr, ntohs(client_addr.sin_port));
                if(add_socket_info(client_socket) == false)
                    close(client_socket);
            }
            if(--num_ready <= 0)
                continue;
        }

        for(int i = 1; i < num_sockets; ++i)
        {
            SocketInfo* socket_info = socket_info_arr[i];
            SOCKET sock = poll_fd_arr[i].fd;
            if(poll_fd_arr[i].revents & POLLIN)
            {
                ret = recv(sock, socket_info->buffer, BUFFER_SIZE, 0);
                if(ret == SOCKET_ERROR)
                {
                    error_display("recv()");
                    remove_socket_info(i);
                }
                else if(ret == 0)
                {
                    remove_socket_info(i);
                }
                else
                {
                    socket_info->recv_bytes = ret;

                    addr_len = sizeof(client_addr);
                    getpeername(sock, (struct sockaddr*)&client_addr, &addr_len);

                    socket_info->buffer[socket_info->recv_bytes] = '\0';
                    char addr[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &client_addr.sin_addr, addr, sizeof(addr));
                    printf("[TCP/%s:%d] %s\n", addr, ntohs(client_addr.sin_port), socket_info->buffer);
                }
            }
            else if(poll_fd_arr[i].revents & POLLOUT)
            {
                ret = send(sock, socket_info->buffer + socket_info->send_bytes, 
                    socket_info->recv_bytes - socket_info->send_bytes, 0);
                if(ret == SOCKET_ERROR)
                {
                    error_display("send()");
                    remove_socket_info(i);
                }
                else
                {
                    socket_info->send_bytes += ret;
                    if(socket_info->recv_bytes == socket_info->send_bytes)
                        socket_info->recv_bytes = socket_info->send_bytes = 0;
                }
            }
        }
    }

    close(listen_socket);

    return 0;
}

bool add_socket_info(SOCKET sock)
{
    if(num_sockets >= FD_SETSIZE)
    {
        printf("[Error] full socket size.\n");
        return false;
    }

    SocketInfo* sock_info = new SocketInfo;
    if(sock_info == NULL)
    {
        printf("[Error] memory error\n");
        return false;
    }
    sock_info->recv_bytes = 0;
    sock_info->send_bytes = 0;
    socket_info_arr[num_sockets] = sock_info;

    poll_fd_arr[num_sockets].fd = sock;
    poll_fd_arr[num_sockets].events = POLLIN;

    ++num_sockets;
    return true;
}


void remove_socket_info(int index)
{
    SocketInfo* sock_info = socket_info_arr[index];
    SOCKET sock = poll_fd_arr[index].fd;

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    getpeername(sock, (struct sockaddr*)&client_addr, &addr_len);

    char addr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, addr, sizeof(addr));
    printf("[TCP server] close client: IP addr=%s, Port num=%d\n", addr, ntohs(client_addr.sin_port));

    close(sock);
    delete sock_info;

    if(index != (num_sockets - 1))
    {
        socket_info_arr[index] = socket_info_arr[num_sockets - 1];
        poll_fd_arr[index] = poll_fd_arr[num_sockets - 1];
    }
    --num_sockets;
}
