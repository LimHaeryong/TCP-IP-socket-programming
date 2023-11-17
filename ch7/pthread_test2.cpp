#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

void* test_thread(void* arg)
{
    errno = 0;
    int ret = nice(-20);
    if(ret < 0 && errno != 0)
    {
        perror("nice() in test_thread");
        exit(1);
    }

    while(true)
    {
        write(1, ".", 1);
    }

    return 0;
}

int main()
{
    int num_of_processors = sysconf(_SC_NPROCESSORS_ONLN);

    for(int i = 0; i < num_of_processors; ++i)
    {
        pthread_t tid;
        pthread_create(&tid, NULL, test_thread, NULL);
    }

    errno = 0;
    int ret = nice(19);
    if(ret < 0 && errno != 0)
    {
        perror("nice() in main");
        exit(1);
    }

    sleep(1);
    while(true)
    {
        write(1, "*", 1);
    }

    return 0;
}