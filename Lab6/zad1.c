#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#define BUFFER_LENGTH 64

double f(double x) {
    return 4 / (x * x + 1);
}

void process_calculate(double start, double end, int fd[2], double dx) {
    close(fd[0]);
    double result = 0;

    double current_x = start; 

    while (current_x <= end) {
        result += dx * f(current_x);
        current_x += dx;
    }

    write(fd[1], &result, sizeof(result));
    close(fd[1]);
}

double calculate_with_processes(int k, double dx) {
    double process_segment_length = 1.0 / (double)k;
    double result = 0;

    int* pipe_read_points = malloc(sizeof(int) * k);

    for (int i = 0; i < k; i++) {
        int fd[2];
        pipe(fd);

        pipe_read_points[i] = fd[0];

        pid_t child = fork();
        
        if (child == 0) {
            process_calculate(i * process_segment_length, (i + 1) * process_segment_length, fd, dx);
            exit(0);
        }
        close(fd[1]);
    }

    while (wait(NULL) > 1);

    for (int i = 0; i < k; i++) {
        double buffer;
        read(pipe_read_points[i], &buffer, sizeof(buffer));
        close(pipe_read_points[i]);
        result += buffer;
    }

    free(pipe_read_points);
    return result;
}

int main(int argc, char** argv) {
    printf("Liczba rdzeni logicznych w komputerze: %ld\n", sysconf(_SC_NPROCESSORS_ONLN));

    struct timeval start, end;

    double dx = atof(argv[1]);
    int n = atoi(argv[2]);

    for (int process_number = 1; process_number <= n; process_number++) {
        gettimeofday(&start, NULL);

        double integration_result = calculate_with_processes(process_number, dx);

        gettimeofday(&end, NULL);

        printf("Liczba procesów: %d; wynik: %f; czas: %f\n", process_number, integration_result, end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec) / 1e6);
    }

    return 0;
}