// Ze slajdów - wypisać pid, a także adres i wartość zmiennej 'i'

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    pid_t pid;
    
    for (int i = 0; i < 10; i++) {
        pid = fork();
        

        if (pid == 0) {
            printf("PID: %d, Adres i: %p, Wartość i: %d\n", getpid(), &i, i);
            return 0;
        } 
    }

    wait(NULL);
    return 0;
}

