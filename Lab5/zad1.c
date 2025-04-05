#define _POSIX_C_SOURCE 200112L
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>

void handler(int signum) {
    printf("Otrzymałem sygnał SIGUSR1\n");
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Podano nieprawidłową liczbę argumentów\n");
        return 0;
    }

    if (strcmp(argv[1], "none") == 0) {
        printf("Nie zmieniam reakcji na sygnał\n");
    } else if (strcmp(argv[1], "ignore") == 0) {
        signal(SIGUSR1, SIG_IGN);
    } else if (strcmp(argv[1], "handler") == 0) {
        signal(SIGUSR1, handler);
    } else if (strcmp(argv[1], "mask") == 0) {
        sigset_t usr_signal;
        sigemptyset(&usr_signal);
        sigaddset(&usr_signal, SIGUSR1);
        sigprocmask(SIG_SETMASK, &usr_signal, NULL);
    }

    raise(SIGUSR1);

    sigset_t sig_pending; 
    sigpending(&sig_pending);

    if (sigismember(&sig_pending, SIGUSR1)) {
        printf("Sygnał jest maskowany i oczekuje na odblokowanie\n");
    }

    return 0;   
}