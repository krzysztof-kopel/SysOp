#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

int main() {
    pid_t pid;
    int fd[2];
    char buf[256];
    pipe(fd);
    pid = fork();

    if (pid == 0) {
        close(fd[0]);
        int w = write(fd[1], "1234567890", 10);
        sleep(5);
        close(fd[1]);
        return 0;
    }

    close(fd[1]);
    int w = read(fd[0], buf, 10);
    close(fd[0]);
    buf[w] = 0;

    printf("%s\n", buf);
    fflush(stdout);

    return 0;
}