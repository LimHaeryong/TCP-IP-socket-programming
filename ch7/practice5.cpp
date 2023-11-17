#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define BUFFER_SIZE 10

pthread_cond_t write_cv;
pthread_cond_t read_cv;
pthread_mutex_t write_mutex;
pthread_mutex_t read_mutex;

int write_done = 0;
int read_done = 0;
int buffer[BUFFER_SIZE];

void* write_thread(void* arg)
{
    for(int k = 1; k <= 500; ++k)
    {
        pthread_mutex_lock(&read_mutex);
        while(read_done < 2)
        {
            pthread_cond_wait(&read_cv, &read_mutex);
        }
        read_done = 0;
        pthread_mutex_unlock(&read_mutex);

        memset(buffer, 0, sizeof(buffer));

        for(int i = 0; i < BUFFER_SIZE; ++i)
            buffer[i] = k;
        
        pthread_mutex_lock(&write_mutex);
        write_done = 2;
        pthread_mutex_unlock(&write_mutex);
        pthread_cond_broadcast(&write_cv);
    }

    return NULL;
}

void* read_thread(void* arg)
{
    while(true)
    {
        pthread_mutex_lock(&write_mutex);
        while(write_done == 0)
        {
            pthread_cond_wait(&write_cv, &write_mutex);
        }
        write_done -= 1;
        pthread_mutex_unlock(&write_mutex);

        pthread_mutex_lock(&read_mutex);
        
        printf("Thread %4d:\t", (int)pthread_self());
        for(int i = 0; i < BUFFER_SIZE; ++i)
            printf("%3d ", buffer[i]);
        printf("\n");

        read_done += 1;
        pthread_mutex_unlock(&read_mutex);
        pthread_cond_signal(&read_cv);
    }   

    return NULL;
}

int main()
{
    pthread_cond_init(&write_cv, NULL);
    pthread_cond_init(&read_cv, NULL);
    pthread_mutex_init(&write_mutex, NULL);
    pthread_mutex_init(&read_mutex, NULL);

    pthread_t tid[3];
    pthread_create(&tid[0], NULL, write_thread, NULL);
    pthread_create(&tid[1], NULL, read_thread, NULL);
    pthread_create(&tid[2], NULL, read_thread, NULL);

    pthread_mutex_lock(&read_mutex);
    read_done = 2;
    pthread_mutex_unlock(&read_mutex);
    pthread_cond_signal(&read_cv);

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    pthread_join(tid[2], NULL);

    pthread_cond_destroy(&write_cv);
    pthread_cond_destroy(&read_cv);
    pthread_mutex_destroy(&write_mutex);
    pthread_mutex_destroy(&read_mutex);


    return 0;
}