#include <stdio.h>
#include <time.h>
#include <pthread.h>

pthread_t watek01, watek02;

void* fun_watka(void* cos) {
    char* message = (char*)cos;
    printf("%s\n", message);
}

int main(void) {
    int arg[10];

    pthread_create(&watek01, NULL, &fun_watka, "A");
    pthread_create(&watek02, NULL, &fun_watka, "B");

    return 0;
}
