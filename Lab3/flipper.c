#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1000

// Robimy tylko zadanie Pliki lustrzane

int reverse_a_file(char* input_path, char* output_path) {
    int file_input = open(input_path, O_RDONLY);

    int file_output = open(output_path, O_WRONLY | O_CREAT | O_TRUNC, 0400);

    if (file_input < 0) {
        return 1;
    }

    char buffer;

    off_t input_file_size = lseek(file_input, 0, SEEK_END);

    lseek(file_input, 0, SEEK_SET);

    int current_line_length = 0; // Ile znaczących elementów jest w line?

    char line[MAX_LINE_LENGTH];

    for (int i = 0; i <= input_file_size; i++) {

        if (read(file_input, &buffer, 1) < 0) {
            return 1;
        }

        if (buffer == '\n' || i == input_file_size) {
            for (int j = current_line_length - 1; j >= 0; j--) {
                write(file_output, &line[j], 1);
            }
            if (i != input_file_size) {
                write(file_output, "\n", 1);
            }
            current_line_length = 0;
        } else {
            line[current_line_length++] = buffer;
        }
        
    }

    close(file_input);
    close(file_output);
    return 0;
}

int is_text_file(char* file_name) {
    char* extension = strrchr(file_name, '.');
    if (extension == NULL || extension == file_name) {
        return 0;
    }

    return strcmp(extension, ".txt") == 0;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Podano nieprawidłową liczbę argumentów.");
        return 1;
    }

    DIR* input_directory = opendir(argv[1]);
    if (input_directory == NULL) {
        printf("Nie znaleziono katalogu wejściowego");
        return 1;
    }

    DIR* output_directory = opendir(argv[2]);
    if (output_directory == NULL) {
        mkdir(argv[2], 0700);
        output_directory = opendir(argv[2]);
    }


    struct dirent* current_file;
    while ((current_file = readdir(input_directory)) != NULL) {
        if (!is_text_file(current_file->d_name)) {
            continue;
        }

        char input_file_path[MAX_LINE_LENGTH];
        snprintf(input_file_path, MAX_LINE_LENGTH, "%s/%s", argv[1], current_file->d_name);

        char output_file_path[MAX_LINE_LENGTH];
        snprintf(output_file_path, MAX_LINE_LENGTH, "%s/%s", argv[2], current_file->d_name);

        if (reverse_a_file(input_file_path, output_file_path) == 1) {
            printf("Wystąpił błąd w czasie odczytywania pliku %s\n", current_file->d_name);
            remove(output_file_path);
        }
    }


    closedir(input_directory);
    closedir(output_directory);
    return 0;
}