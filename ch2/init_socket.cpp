#include "../common.h"

int f(int x)
{
    if(x >= 0)
    {
        errno = 0;
        return 0;
    }
    else
    {
        errno = EINVAL;
        return SOCKET_ERROR;
    }
}

int main(int argc, char* argv[])
{   
    // SOCK_DGRAM = UDP, SOCK_STREAM = TCP
    // AF_INET = IPv4, AF_INET6 = IPv6
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == INVALID_SOCKET) error_quit("socket()");

    printf("[INFO] socket init successed\n");

    close(sock);

    int retval = f(-100);
    if(retval < 0) error_quit("f()");

    return 0;
}