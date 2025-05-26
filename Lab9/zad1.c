#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

struct DataPack {
    double start;
    double end;
    double dx;
    double result;
};

double f(double x) {
    return 4 / (x * x + 1);
}

void* thread_calculate(void* data_pointer) {
    struct DataPack* data = (struct DataPack*)data_pointer;
    double result = 0;

    double current_x = data->start; 

    while (current_x <= data->end) {
        result += data->dx * f(current_x);
        current_x += data->dx;
    }

    data->result = result;
    return NULL;
}

double calculate_with_threads(int k, double dx) {
    double thread_segment_length = 1.0 / (double)k;
    double result = 0;

    struct DataPack* thread_data_packs = malloc(sizeof(struct DataPack) * k);

    double current = 0;
    int i = 0;

    for (int i = 0; i < k; i++) {
        thread_data_packs[i].start = current;
        current += thread_segment_length;
        thread_data_packs[i].end = current;
        thread_data_packs[i].dx = dx;
    }

    pthread_t* threads = malloc(sizeof(pthread_t) * k);

    for (int i = 0; i < k; i++) {
        
        pthread_create(&threads[i], NULL, &thread_calculate, &thread_data_packs[i]);
    }

    for (int i = 0; i < k; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < k; i++) {
        result += thread_data_packs[i].result;
    }

    free(thread_data_packs);
    free(threads);
    return result;
}

int main(int argc, char** argv) {
    printf("Liczba rdzeni logicznych w komputerze: %ld\n", sysconf(_SC_NPROCESSORS_ONLN));

    struct timeval start, end;

    double dx = atof(argv[1]);
    int n = atoi(argv[2]);

    for (int thread_number = 1; thread_number <= n; thread_number++) {
        gettimeofday(&start, NULL);

        double integration_result = calculate_with_threads(thread_number, dx);

        gettimeofday(&end, NULL);

        printf("Liczba wątków: %d; wynik: %f; czas: %f\n", thread_number, integration_result, end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec) / 1e6);
    }

    return 0;
}