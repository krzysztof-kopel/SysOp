#include <sys/ipc.h>
#include <sys/msg.h>

struct pakiet {
    long typ;
    int zawartosc;
};

int main(void) {
    int i;
    struct pakiet o1;

    key_t klucz = ftok("plik1.txt", 'p');
    int id_kolejki_kom = msgget(klucz, IPC_CREAT | 0600);
    for (int i = 0; i < 5; i++) {
        o1.typ = (i % 2) + 1;
        o1.zawartosc = i;
        msgsnd(id_kolejki_kom, &o1, sizeof(o1), 0);
    }

    return 0;
}
