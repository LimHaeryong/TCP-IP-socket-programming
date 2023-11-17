#include "../common.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9000
#define BUFFER_SIZE 50

int main()
{
    int ret;

    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket == INVALID_SOCKET) error_quit("socket()");

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
    server_addr.sin_port = htons(SERVER_PORT);
    ret = connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret == SOCKET_ERROR) error_quit("connect()");

    char buffer[BUFFER_SIZE];
    const char* test_data[] = {
        "test1~~..",
        "test2 hello",
        "test3 asdasdasd"
    };
    int len, len_network;

    for(int i = 0; i < 3; ++i)
    {
        len = (int)strlen(test_data[i]);
        len_network = htonl(len);
        strncpy(buffer, test_data[i], len);

        // send header
        ret = send(client_socket, (char*)&len_network, sizeof(int), 0);
        if(ret == SOCKET_ERROR)
        {
            error_display("send()");
            break;
        }

        // send data
        ret = send(client_socket, buffer, len, 0);
        if(ret == SOCKET_ERROR)
        {
            error_display("send()");
            break;
        }
        printf("[TCP client] send %d+%d bytes.\n", (int)(sizeof(int)), ret);
    }

    close(client_socket);
    return 0;
}