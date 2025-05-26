// Pamięć wspólna -> System V
// Semafory -> POSIX
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define MAX_MESSAGE_NUMBER 30
#define MESSAGE_LENGTH 10
#define KEY_CONST 1
#define SEMAPHORE_NAME "queue_access_sem"

struct queue {
    char content[MAX_MESSAGE_NUMBER][MESSAGE_LENGTH];
    int size;
    int first;
    int last;
};

int add(struct queue* q, char* message);

char* get(struct queue* q);

void init_queue(struct queue* q);

int is_empty(struct queue* q);
