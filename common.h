#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <poll.h>
#include <sys/epoll.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>

typedef int SOCKET;
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1

void error_quit(const char* msg)
{
    char* msg_buffer = strerror(errno);
    printf("[%s] %s\n", msg, msg_buffer);
    exit(1);
}

void error_display(const char* msg)
{
    char* msg_buffer = strerror(errno);
    printf("[%s] %s\n", msg, msg_buffer);
}

void error_display(int error_code)
{
    char *msg_buffer = strerror(error_code);
    printf("[error] %s", msg_buffer);
}