#include "../common.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9000
#define BUFFER_SIZE 512

int main(int argc, char* argv[])
{

    int ret;

    SOCKET client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(client_socket == INVALID_SOCKET) error_quit("socket()");

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
    server_addr.sin_port = htons(SERVER_PORT);

    struct sockaddr_in peer_addr;
    socklen_t addr_len;
    char buffer[BUFFER_SIZE + 1];

    if(argc == 1) error_quit("argc");
    int* int_array = new int[argc - 1];

    for(int i = 1; i < argc; ++i)
        int_array[i - 1] = atoi(argv[i]);

    int size = sizeof(int) * (argc - 1);

    memcpy(buffer, int_array, size);

    ret = sendto(client_socket, buffer, size, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret == SOCKET_ERROR) error_quit("sendto()");

    printf("[UDP client] send %d bytes.\n", ret);
    addr_len = sizeof(peer_addr);
    ret = recvfrom(client_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&peer_addr, &addr_len);
    if(ret == SOCKET_ERROR) error_quit("recvfrom()");
    if(memcmp(&peer_addr, &server_addr, sizeof(peer_addr)))
    {
        printf("[Error] wrong data.\n");
        return 0;
    }

    int result;
    memcpy(&result, buffer, sizeof(int));

    printf("[UDP client] received %d bytes.\n", ret);
    printf("[UDP client] received data : %d\n", result);

    close(client_socket);
    delete[] int_array;

    return 0;
}