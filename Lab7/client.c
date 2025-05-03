#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define MSG_SIZE 256
#define INIT 2
#define NORMAL 1

int client_msqid;

struct msg_t {
    long type;
    char message[MSG_SIZE];
};

void cleanup() {
    msgctl(client_msqid, IPC_RMID, NULL);
    printf("Usunięto kolejkę klienta\n");
    exit(0);
}

int main(int argc, void** argv) {
    if (argc != 2) {
        printf("Podano nieprawidłową liczbę argumentów\n");
        return -1;
    }

    signal(SIGINT, cleanup);

    srand(time(NULL));
    key_t key = ftok("plik1.txt", rand());
    client_msqid = msgget(key, IPC_CREAT | 0666);
    printf("ID kolejki klienta: %d\n", client_msqid);


    int server_msqid = atoi(argv[1]);

    struct msg_t init;
    init.type = INIT;
    sprintf(init.message, "%d", key);

    msgsnd(server_msqid, &init, sizeof(init.message), IPC_NOWAIT);

    if (msgrcv(client_msqid, &init, sizeof(init.message), INIT, 0) > 0) {
        printf("Został mi przydzielony numer klienta %d\n", atoi(init.message));
    } else {
        printf("Wystąpil błąd przy odczycie\n");
        perror("msgrcv");
        raise(SIGINT);
    }

    pid_t receiver = fork();

    if (receiver == 0) {
        struct msg_t incoming;
        incoming.type = NORMAL;
        while (msgrcv(client_msqid, &incoming, sizeof(incoming.message), NORMAL, 0) > 0) {
            printf("Odebrano wiadomość: '%s'\n", incoming.message);
        }
    } else {
        struct msg_t outgoing;
        outgoing.type = NORMAL;
        
        while (1) {
            fgets(outgoing.message, 256, stdin);

            outgoing.message[strlen(outgoing.message) - 1] = 0;

            msgsnd(server_msqid, &outgoing, sizeof(outgoing.message), IPC_NOWAIT);
        }
    }

    return 0;
}
