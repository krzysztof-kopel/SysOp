#include <stdlib.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>

#define MAX_MESSAGE_NUMBER 100
#define MESSAGE_LENGTH 10

// Pomysł chyba trochę nadmiernie skomplikowany, nie będzie już kontynuowany

struct msg {
    pid_t sender_pid;
    char message[MESSAGE_LENGTH];
};

void cleanup() {
    shmctl(shared_memory_id, IPC_RMID, NULL);
    printf("Usunięto segment pamięci współdzielonej\n");
    sem_close(mem_sem_address);
    sem_close(capacity_sem_addres);
    sem_unlink("capacity");
    sem_unlink("memory");
    printf("Usunięto semafory\n");
    exit(0);
}

void send_random_numbers() {
    while (1) {
        struct msg outgoing;
        outgoing.sender_pid = getpid();
        for (int j = 0; j < MESSAGE_LENGTH; j++) {
            strcat(outgoing.message, rand() % (122 - 97) + 97); // 'a' -> 97, 'z' -> 122
        }
        sem_wait(mem_sem_address);

        struct msg* address_to_read = (struct msg*)shmat(shared_memory_id, NULL, NULL);
        strcat(address_to_read, outgoing);
        int current_full;
        sem_getvalue(capacity_sem_addres, &current_full);
        // Tu kontynuować
    }
}

int shared_memory_id;
sem_t* mem_sem_address;
sem_t* capacity_sem_addres;

int main(int argc, char** argv) {
    signal(SIGINT, cleanup);

    if (argc != 2) {
        printf("Podano nieprawidłową liczbę argumentów\n");
        return -1;
    }

    int number_of_users = atoi(argv[1]);

    srand(time(NULL));
    key_t key = ftok("porada.txt", rand());
    shared_memory_id = shmget(key, MAX_MESSAGE_NUMBER * sizeof(struct msg), IPC_CREAT | 0666);

    mem_sem_address = sem_open("memory", O_CREAT, 0666, 1);
    capacity_sem_addres = sem_open("capacity", O_CREAT, 0666, 0);


    for (int i = 0; i < number_of_users; i++) {
        pid_t child = fork();

        if (child == 0) {
            signal(SIGINT, SIG_DFL);
            send_random_messages();
        }
    }
    return 0;
}