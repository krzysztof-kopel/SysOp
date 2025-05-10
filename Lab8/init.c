#include "common.h"

int shm_id;
struct queue* shm_queue_pointer;
sem_t* semaphore_pointer;

void cleanup() {
    if (shm_id != -1) {
        shmdt(shm_queue_pointer);
        shmctl(shm_id, IPC_RMID, NULL);
    }
    
    if (semaphore_pointer != NULL) {
        sem_close(semaphore_pointer);
        sem_unlink("queue_access_sem");
    }

    printf("Usunięto pamięć wspólną i semafor.\n");
    exit(0);
}

int main() {
    signal(SIGINT, cleanup);

    struct queue queue;
    init_queue(&queue);

    int key = ftok("porada.txt", KEY_CONST);
    shm_id = shmget(key, sizeof(struct queue), IPC_CREAT | 0666);
    shm_queue_pointer = (struct queue*)shmat(shm_id, NULL, 0);
    memcpy(shm_queue_pointer, &queue, sizeof(struct queue));

    semaphore_pointer = sem_open(SEMAPHORE_NAME, O_CREAT, 0666, 1);

    if (shm_id == -1 || shm_queue_pointer == (void*) - 1 || semaphore_pointer == SEM_FAILED) {
        printf("Nastąpił błąd przy tworzeniu segmentu pamięci wspólnej lub semaforu.\n");
        cleanup();
    }

    printf("Utworzono segment pamięci wspólnej i semafor.\n");
    pause();
}