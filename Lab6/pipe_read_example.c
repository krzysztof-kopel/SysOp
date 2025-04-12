#define _POSIX_C_SOURCE 200112L
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int ipar, char *tab[]) {
    int w1;
    char buf[1024];

    w1 = open("potok", O_RDONLY);

    read(w1, buf, 10);

    printf("otrzymano: %s\n", buf);

    close(w1);

    return 0;
}