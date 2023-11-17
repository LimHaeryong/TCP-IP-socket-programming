#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

struct Point3D
{
    int x, y, z;
};

void *test_thread(void* arg)
{
    sleep(1);
    Point3D* pt = (Point3D*)arg;
    printf("Running test_thread() %lu: %d, %d, %d\n", pthread_self(), pt->x, pt->y, pt->z);

    delete pt;

    return NULL;
}

int main()
{
    int ret;

    pthread_t tid1;
    Point3D* pt1 = new Point3D{10, 20, 30};
    ret = pthread_create(&tid1, NULL, test_thread, pt1);
    if(ret != 0) return 1;

    pthread_t tid2;
    Point3D* pt2 = new Point3D{40, 50, 60};
    ret = pthread_create(&tid2, NULL, test_thread, pt2);
    if(ret != 0) return 1;

    printf("Running main() %lu\n", pthread_self());
    //sleep(2);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    return 0;
}