#define _POSIX_C_SOURCE 200112L
#include <signal.h>
#include <stddef.h>
#include <unistd.h>
#include <stdio.h>

int requests = 0;
int mode = 0;

void handler(int sig_no, siginfo_t* siginfo, void* p3) { // Ten p3 chyba do niczego nie służy, ale wzorowałem się programem ze slajdów
    mode = siginfo->si_value.sival_int;
    requests++;
    kill(siginfo->si_pid, SIGUSR1);
}

void sigint_handler() {
    printf("Wciśnięto CTRL+C\n");
}

int main() {
    printf("Moje PID to: %d\n", getpid());

    sigset_t set, empty_set;
    sigemptyset(&empty_set);

    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigprocmask(SIG_SETMASK, &set, NULL);

    struct sigaction sigact;
    sigact.sa_sigaction = handler;
    sigact.sa_flags = SA_SIGINFO;
    sigemptyset(&sigact.sa_mask);
    sigaction(SIGUSR1, &sigact, NULL);

    while (mode != 5) {
        sigsuspend(&empty_set);

        switch (mode) {
            case 1:
                printf("Liczba otrzymanych żądań: %d\n", requests);
                break;

            case 2:
                for (int i = 0; mode == 2; i++) {
                    printf("%d\n", i);
                    sleep(1);
                }
                break;

            case 3:
                signal(SIGINT, SIG_IGN);
                break;

            case 4:
                signal(SIGINT, sigint_handler);
                break;

            case 5:
                break;

            default:
                printf("Otrzymano komendę spoza zakresu 0-5\n");
                break;
        }
    }

    printf("Koniec działania catchera\n");
    return 0;
}