#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

struct segment {
    int start;
    int end;
};

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Podano nieprawidłową liczbę argumentów\n");
        return -1;
    }

    int start = atoi(argv[1]);
    int end = atoi(argv[2]);

    if (mkfifo("data_pipe", 0666) < 0 || mkfifo("result_pipe", 0666) < 0) {
        printf("Wystąpił błąd przy tworzeniu potoku\n");
        return -1;
    }    

    int data_pipe = open("data_pipe", O_WRONLY);

    struct segment seg;
    seg.start = start;
    seg.end = end;

    write(data_pipe, &seg, sizeof(seg));
    close(data_pipe);

    int result_pipe = open("result_pipe", O_RDONLY);
    double result;
    read(result_pipe, &result, sizeof(result));
    close(result_pipe);

    printf("Obliczona wartość: %f\n", result);
    remove("data_pipe");
    remove("result_pipe");
    return 0;
}
