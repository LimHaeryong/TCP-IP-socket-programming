#include <stdio.h>
#include <pthread.h>

#define MAXCNT 100000000
int count = 0;
pthread_mutex_t mutex;

void* test_thread1(void* arg)
{
    for(int i = 0; i < MAXCNT; ++i)
    {
        pthread_mutex_lock(&mutex);
        count += 2;
        pthread_mutex_unlock(&mutex);
    }

    return 0;
}

void* test_thread2(void* arg)
{
    for(int i = 0; i < MAXCNT; ++i)
    {
        pthread_mutex_lock(&mutex);
        count -= 2;
        pthread_mutex_unlock(&mutex);
    }

    return 0;
}

int main()
{
    pthread_mutex_init(&mutex, NULL);

    pthread_t tid[2];
    pthread_create(&tid[0], NULL, test_thread1, NULL);
    pthread_create(&tid[1], NULL, test_thread2, NULL);

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);

    pthread_mutex_destroy(&mutex);

    printf("count = %d\n", count);

    return 0;
}