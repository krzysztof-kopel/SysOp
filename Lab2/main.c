#include <stdio.h>
#include <stdlib.h>
#define MAX_ITER 100

int test_collatz_convergence(int input, int max_iter, int *steps);

int main(int argc, char** argv) {
    if (argc != 2 || atoi(argv[1]) <= 0) {
        printf("Niepoprawne dane wejściowe\n");
        return 1;
    }

    int input = atoi(argv[1]);

    int steps[MAX_ITER];
    int collatz_result = test_collatz_convergence(input, MAX_ITER, steps);

    if (collatz_result > 0) {
        printf("Liczba operacji: %d\n", collatz_result);

        printf("Kolejne kroki:\n");
        for (int i = 0; i <= collatz_result; i++) {
            printf("%d\n", steps[i]);
        }
    } else {
        printf("Nie udało się osiągnąć 1 w %d operacjach\n", MAX_ITER);
    }
    
    return 0;
}