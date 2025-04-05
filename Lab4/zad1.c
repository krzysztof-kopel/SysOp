#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv) {
    int numberOfProcesses; 
    if (argc != 2 || (numberOfProcesses = atoi(argv[1])) <= 0) {
        printf("Podaną złą liczbę argumentów lub argument nie jest liczbą\n");
        return 1;
    }
    int process_counter = 0;
    pid_t pid;
    for (int i = 0; i < numberOfProcesses; i++) {
        pid = fork(); // Warto sprawdzać, czy faktycznie proces się utworzył
        if (pid == 0) {
            printf("Własny identyfikator: %d, Identyfikator procesu macierzystego: %d\n", getpid(), getppid());
            return 0;
        }

        if (pid > 0) {
            process_counter++;
        }
    }

    for (int i = 0; i < numberOfProcesses; i++) {
        wait(NULL);
    }

    printf("Liczba realnie utworzonych procesów: %d\n", process_counter);
    printf("Liczba procesów: %s\n", argv[1]);
    return 0;
}
