#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define MSG_SIZE 256
#define INIT 100
#define NORMAL 1

int msqid;

struct msg_t {
    long type;
    char message[MSG_SIZE];
};

void cleanup() {
    msgctl(msqid, IPC_RMID, NULL);
    printf("Usunięto kolejkę serwera\n");
    exit(0);
}

int main() {
    signal(SIGINT, cleanup);

    srand(time(NULL));
    key_t key = ftok("plik.txt", rand());
    msqid = msgget(key, IPC_CREAT | 0666);
    printf("ID kolejki serwera: %d\n", msqid);

    int current_client_number = 0;
    int client_keys[100];

    struct msg_t msg;

    while (1) {
        if (msgrcv(msqid, &msg, sizeof(msg.message), 0, 0) > 0) {
            if (msg.type == INIT) {

                key_t client_key = (key_t)atoi(msg.message);
                int client_qid = msgget(client_key, 0);
                client_keys[current_client_number++] = client_qid;
                printf("Zarejestrowano klienta numer %d z kolejką o kluczu: %ld\n", current_client_number, (long)client_key);

                struct msg_t reply;
                reply.type = INIT;
                sprintf(reply.message, "%d", current_client_number);
                msgsnd(client_qid, &reply, sizeof(reply.message), 0);

            } else {

                struct msg_t outgoing;
                outgoing.type = NORMAL;
                strcpy(outgoing.message, msg.message);

                for (int i = 0; i < current_client_number; i++) {
                    if (i == msg.type - 1) {
                        continue;
                    }
                    msgsnd(client_keys[i], &outgoing, sizeof(outgoing.message), IPC_NOWAIT);
                }
                printf("Rozesłano wiadomość '%s' do wszystkich klientów.\n", msg.message);
            }
            
        } else {
            printf("Wystąpil błąd przy odczycie\n");
            perror("msgrcv");
            raise(SIGINT);
        }
    }
    return 0;
}
