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
    int retval = f(-100);
    if(retval < 0) error_quit("f()");

    return 0;
}