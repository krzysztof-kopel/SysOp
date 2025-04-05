int collatz_conjecture(int input) {
    if (input % 2 == 0) {
        return input / 2;
    }
    return 3 * input + 1;
}

int test_collatz_convergence(int input, int max_iter, int *steps) {
    int i = 0;
    steps[0] = input;
    while (i < max_iter && input != 1) {
        input = collatz_conjecture(input);
        steps[i++ + 1] = input;
    }
    if (input == 1) {
        return i;
    }
    return -1;
}
