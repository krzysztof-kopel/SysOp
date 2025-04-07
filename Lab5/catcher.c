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

void sigint_handler(int sig) {
    printf("Wciśnięto CTRL+C\n");
}

int main() {
    printf("Moje PID to: %d\n", getpid());

    sigset_t empty_set;
    sigemptyset(&empty_set);

    struct sigaction sigact;
    sigact.sa_sigaction = handler;
    sigact.sa_flags = SA_SIGINFO;
    sigemptyset(&sigact.sa_mask);
    sigaction(SIGUSR1, &sigact, NULL);

    while (mode != 5) {
        switch (mode) {
            case 1:
                printf("Liczba otrzymanych żądań: %d\n", requests);
                break;

            case 2:
                for (int i = 0; mode == 2; i++) {
                    printf("%d\n", i);
                    sleep(1);
                }
                continue; // Gdyby tutaj był break, to sygnał przerywający równy 3 albo 4 nie 
                // zmieniałby poprawnie zachowania SIGINT

            case 3:
                signal(SIGINT, SIG_IGN);
                break;

            case 4:
                struct sigaction sigact4; 
                sigact4.sa_flags = 0;
                sigemptyset(&sigact4.sa_mask);
                sigact4.sa_handler = sigint_handler; 

                sigaction(SIGINT, &sigact4, NULL);
                break;

            case 5:
                break;

            case 0:
                break;

            default:
                printf("Otrzymano komendę spoza zakresu 1-5\n");
                break;
        }

        sigsuspend(&empty_set);
    }

    printf("Koniec działania catchera\n");
    return 0;
}