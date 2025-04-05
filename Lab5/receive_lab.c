#define _POSIX_C_SOURCE 200112L
#include <signal.h>
#include <stddef.h>
#include <stdio.h>

void obsluga_v1(int signum, siginfo_t* si, void* p3) {
    printf("Obsługa syngału v1: %d %d , var: %d\n", si->si_pid, si->si_uid, si-> si_value);
}

int main(void) {
    struct sigaction act; 
    printf("pid:%d\n", getpid());

    act.sa_sigaction = obsluga_v1;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);

    pause();
}
