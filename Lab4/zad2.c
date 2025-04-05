#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>

int global = 0;

int main(int argc, char** argv) {
    int error_code;

    if (argc != 2) {
        printf("Podaną złą liczbę argumentów\n");
        return 1;
    }

    printf("Nazwa programu: %s\n", argv[0]);

    int local = 0;

    pid_t pid = fork();
    if (pid == 0) {
        printf("Child process\n");
        global++;
        local++;
        printf("child pid = %d, parent pid = %d\n", getpid(), getppid());
        printf("child's local = %d, child's global = %d\n\n", local, global);
        execl("/bin/ls", "ls", argv[1], NULL);
    } else {
        wait(&error_code);
        printf("\n");
        printf("Parent process\n");
        printf("parent pid = %d, child pid = %d\n", getpid(), pid);
        printf("Child exit code: %d\n", WEXITSTATUS(error_code));
        printf("Parent's local = %d, parent's global = %d\n", local, global);
        return error_code;
    }
}
