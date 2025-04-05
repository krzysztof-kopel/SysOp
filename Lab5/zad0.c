#include <signal.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>

void obsluga(int signum) {
    printf("Obsługa sygnału\n");
}

int main(void) {
    // signal(SIGUSR1, obsluga);
    // raise(SIGUSR1);
    // while (1);
    // return 0;

    sigset_t newmask, oldmask, set;

    sigemptyset(&newmask);
    sigaddset(&newmask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &newmask, &oldmask);

    raise(SIGUSR1);

    sigpending(&set);
    if (sigismember(&set, SIGUSR1) == 1) {
        printf("SIGUSR1 oczekuje na odblokowanie\n");
    }
}

// 
int main(int argc, char** argv) {
    union signal var;

    int pid = atoi(argv[1]);

    var.signal_int = 123;
    sigqueue(pid, SIGUSR1, var);
    return 0;
}

void obsluga_v1(int signum, siginfo_t* si, void* p3) {
    printf("Obsługa syngału v1: %d %d , var: %d\n", si->si_pid, si->si_uid, si-> si_value)
}

int main(void) {
    signal_t set;
    struct sigaction act; 
    printf("pid:%d\n", getpid());

    act.sa_sigaction = obsluga_v1;
    sigemptyset(&act.sa_mask);
    act.sa_flag = S4_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);
}
