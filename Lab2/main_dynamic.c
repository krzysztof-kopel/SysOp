#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#define MAX_ITER 100

int test_collatz_convergence(int input, int max_iter, int *steps);

int main(int argc, char** argv) {
    if (argc != 2 || atoi(argv[1]) <= 0) {
        printf("Niepoprawne dane wejściowe\n");
        return 1;
    }

    int input = atoi(argv[1]);

    int steps[MAX_ITER];
    
    
    void* handle = dlopen("libcollatz.so", RTLD_LAZY);

    int (*test_collatz_convergence)(int, int, int*) = dlsym(handle, "test_collatz_convergence");

    char* error = dlerror();

    if (error != NULL || !test_collatz_convergence) {
        printf("Wystąpił błąd: %s", error);
        return 1;
    }

    int collatz_result = (*test_collatz_convergence)(input, MAX_ITER, steps);
    dlclose(handle);

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