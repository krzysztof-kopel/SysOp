#define _POSIX_C_SOURCE 200112L
#include <signal.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    union sigval var;

    int pid = atoi(argv[1]);

    var.sival_int = 123;
    sigqueue(pid, SIGUSR1, var);
    return 0;
}