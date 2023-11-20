#include "../common.h"

#define REMOTE_IP "255.255.255.255"
#define REMOTE_PORT 9000
#define BUFFER_SIZE 512

int main()
{
    int ret;

    SOCKET sender_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(sender_socket == INVALID_SOCKET) error_quit("socket()");

    int broadcast_enable = 1;
    ret = setsockopt(sender_socket, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));
    if(ret == SOCKET_ERROR) error_quit("setsockopt()");

    struct sockaddr_in remote_addr;
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    inet_pton(AF_INET, REMOTE_IP, &remote_addr.sin_addr);
    remote_addr.sin_port = htons(REMOTE_PORT);

    char buffer[BUFFER_SIZE + 1];
    int len;

    while(true)
    {
        printf("\n[Data to send]:");
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
        printf("[UDP] send %d bytes.\n", ret);

    }

    close(sender_socket);
    return 0;
}