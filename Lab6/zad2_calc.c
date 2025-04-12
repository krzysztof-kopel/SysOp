#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#define dx 0.0001

double f(double x) {
    return 4 / (x * x + 1);
}

struct segment {
    int start;
    int end;
};

int main() {
    int data_pipe = open("data_pipe", O_RDONLY);

    struct segment seg;
    read(data_pipe, &seg, sizeof(seg));
    close(data_pipe);

    double result = 0;
    for (double i = seg.start; i <= seg.end; i += dx) {
        result += f(i) * dx;
    }

    int result_pipe = open("result_pipe", O_WRONLY);
    write(result_pipe, &result, sizeof(result));
    close(result_pipe);

    return 0;
}
