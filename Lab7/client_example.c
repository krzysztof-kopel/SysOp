#define _POSIX_C_SOURCE 200112L
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>

struct pakiet {
    long typ;
    int zawartosc;
};

int main(void) {
    struct pakiet o1;
    key_t klucz = ftok("plik1.txt", 'p');
    int id_kolejki_kom = msgget(klucz, 0600);

    while (1) {
        if (msgrcv(id_kolejki_kom, &o1, sizeof(o1), 1, IPC_NOWAIT) < 0) {
            break;
        }
        printf("Otrzymano: %d\n", o1.zawartosc);
    }

    while (1) {
        if (msgrcv(id_kolejki_kom, &o1, sizeof(o1), 2, IPC_NOWAIT) < 0) {
            break;
        }
        printf("Otrzymano: %d\n", o1.zawartosc);
    }
    msgctl(id_kolejki_kom, IPC_RMID, NULL);
    return 0;
}
