#include <stdio.h>
#include <time.h>
#include <pthread.h>

pthread_mutex_t mutex01 = PTHREAD_MUTEX_INITIALIZER;
pthread_t watek01, watek02;
int i;

void* fun_watka(void* cos) {
    static int a = 10;
    while (1) {
        pthread_mutex_lock(&mutex01);
        a++;
        pthread_mutex_unlock(&mutex01);
        printf("%s %d %d\n", (char*)cos, a, i);
        fflush(stdout);
        sleep(1);
    }
}

int main() {
    pthread_create(&watek01, NULL, &fun_watka, "A");
    pthread_create(&watek02, NULL, &fun_watka, "B");

    sleep(10);
    return 0;
}
