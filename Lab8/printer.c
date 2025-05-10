#include "common.h"

struct queue* queue_pointer;
sem_t* sem_pointer;

void cleanup() {
    if (queue_pointer != NULL) {
        shmdt(queue_pointer);
    }
    
    if (sem_pointer != NULL) {
        sem_close(sem_pointer);
    }

    printf("Odłączono pamięć wspólną i semafor.\n");
    exit(0);
}

void print(char* message) {
    for (int i = 0; i < MESSAGE_LENGTH; i++) {
        printf("%c", message[i]);
        fflush(stdout);
        sleep(1);
    }
    printf("\n");
}

int main() {
    signal(SIGINT, cleanup);
    srand(getpid());

    int key = ftok("porada.txt", KEY_CONST);
    int shm_id = shmget(key, sizeof(struct queue), 0666);
    queue_pointer = (struct queue*)shmat(shm_id, NULL, 0);

    sem_pointer = sem_open(SEMAPHORE_NAME, 0);

    if (shm_id == -1 || queue_pointer == (void*) - 1 || sem_pointer == SEM_FAILED) {
        printf("Nastąpił błąd przy uzyskiwaniu dostępu segmentu pamięci wspólnej lub semaforu.\n");
        cleanup();
    }

    while (1) {
        char message[MESSAGE_LENGTH + 1];

        sem_wait(sem_pointer);
        if (!is_empty(queue_pointer)) {
            strcpy(message, get(queue_pointer));
            message[MESSAGE_LENGTH] = '\0';
        } else {
            strcpy(message, "NONE");
        }
        sem_post(sem_pointer);

        if (strcmp(message, "NONE")) {
            print(message);
        }
        sleep(1);
    }
}