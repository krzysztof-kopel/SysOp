#define _POSIX_C_SOURCE 200112L
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void handler(int sig) {
    printf("Otrzymałem powiadomienie o odebraniu sygnału.\n");
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Podano nieprawidłową liczbę argumentów\n");
        return -1;
    }

    signal(SIGUSR1, handler);

    union sigval value_to_send;
    value_to_send.sival_int = atoi(argv[2]);

    sigqueue(atoi(argv[1]), SIGUSR1, value_to_send);

    pause();

    printf("Kończę pracę\n");
    return 0;
}