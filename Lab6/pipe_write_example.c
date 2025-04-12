#define _POSIX_C_SOURCE 200112L
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
    int w1;
    
    mkfifo("potok", 0666);

    w1 = open("potok", O_WRONLY | O_TRUNC);

    write(w1, "1234567890", 10);

    close(w1);

    return 0;
}