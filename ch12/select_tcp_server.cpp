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

int num_sockets = 0;
struct SocketInfo* socket_info_arr[FD_SETSIZE];

bool add_socket_info(SOCKET sock);
void remove_socket_info(int index);
int get_next_fd(SOCKET sock);

int main(int argc, char* argv[])
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

    int flags = fcntl(listen_socket, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(listen_socket, F_SETFL, flags);

    fd_set read_set, write_set;
    int num_ready;
    SOCKET client_socket;
    struct sockaddr_in client_addr;
    socklen_t addr_len;

    while(true)
    {
        FD_ZERO(&read_set);
        FD_ZERO(&write_set);
        FD_SET(listen_socket, &read_set);
        for(int i = 0; i < num_sockets; ++i)
        {
            if(socket_info_arr[i]->recv_bytes > socket_info_arr[i]->send_bytes)
                FD_SET(socket_info_arr[i]->sock, &write_set);
            else
                FD_SET(socket_info_arr[i]->sock, &read_set);
        }

        num_ready = select(get_next_fd(listen_socket), &read_set, &write_set, NULL, NULL);
        if(num_ready == SOCKET_ERROR) error_quit("select()");

        if(FD_ISSET(listen_socket, &read_set))
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

        for(int i = 0; i < num_sockets; ++i)
        {
            SocketInfo* socket_info = socket_info_arr[i];
            if(FD_ISSET(socket_info->sock, &read_set))
            {
                ret = recv(socket_info->sock, socket_info->buffer, BUFFER_SIZE, 0);
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
                    getpeername(socket_info->sock, (struct sockaddr*)&client_addr, &addr_len);

                    socket_info->buffer[socket_info->recv_bytes] = '\0';
                    char addr[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &client_addr.sin_addr, addr, sizeof(addr));
                    printf("[TCP/%s:%d] %s\n", addr, ntohs(client_addr.sin_port), socket_info->buffer);
                }
            }
            else if(FD_ISSET(socket_info->sock, &write_set))
            {
                ret = send(socket_info->sock, socket_info->buffer + socket_info->send_bytes, 
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

    sock_info->sock = sock;
    sock_info->recv_bytes = 0;
    sock_info->send_bytes = 0;
    socket_info_arr[num_sockets++] = sock_info;
    return true;
}


void remove_socket_info(int index)
{
    SocketInfo* sock_info = socket_info_arr[index];

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    getpeername(sock_info->sock, (struct sockaddr*)&client_addr, &addr_len);

    char addr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, addr, sizeof(addr));
    printf("[TCP server] close client: IP addr=%s, Port num=%d\n", addr, ntohs(client_addr.sin_port));

    close(sock_info->sock);
    delete sock_info;

    if(index != (num_sockets - 1))
    {
        socket_info_arr[index] = socket_info_arr[num_sockets - 1];
    }
    --num_sockets;
}

int get_next_fd(SOCKET sock)
{
    int next_fd = sock;
    for(int i = 0; i < num_sockets; ++i)
    {
        if(socket_info_arr[i]->sock > next_fd)
        {
            next_fd = socket_info_arr[i]->sock;
        }
    }
    ++next_fd;

    return next_fd;
}