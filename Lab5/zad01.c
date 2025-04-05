#define _POSIX_C_SOURCE 199309L
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void handler(int signal) {
    printf("CTRL+C\n");
}

int main() {
    signal(SIGINT, handler);

    while (1);
    return 0;
}
