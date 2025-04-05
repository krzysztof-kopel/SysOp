#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>


int main() {
    int i = 0;

    pid_t pid = fork();

    if (pid == 0) {
        while (i > -100) {
            printf("dziecko -> i: %d, adres: %p\n", i--, &i);
        }
    } else {
        while (i < 100) {
            printf("rodzic -> i: %d, adres: %p\n", i++, &i);
        }
    }
    return 0;
}
