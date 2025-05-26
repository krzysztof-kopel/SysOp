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

void send_next_message() {
    char buffer[MESSAGE_LENGTH + 1] = ""; // +1 żeby było miejsce na \0
    for (int i = 0; i < MESSAGE_LENGTH; i++) {
        buffer[i] = (rand() % ('z' - 'a' + 1)) + 'a';
    }
    buffer[MESSAGE_LENGTH] = '\0';

    int queueFullFlag = 0;
    while (!queueFullFlag) {
        sem_wait(sem_pointer);
        queueFullFlag = add(queue_pointer, buffer);
        sem_post(sem_pointer);
        sleep(1);
    }

    printf("Wysyłam wiadomość %s do drukarek.\n", buffer);
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
        send_next_message();
        sleep(rand() % 5 + 1);
    }
}