#include "common.h"

int main(void) {
    int status, gniazdo;
    struct sockaddr_in ser, cli;
    char buf[200];

    gniazdo = socket(AF_INET, SOCK_STREAM, 0);
    if (gniazdo == -1) {
        printf("Błąd socket\n");
        return 0;
    }

    memset(&ser, 0, sizeof(ser));
    ser.sin_family = AF_INET;
    ser.sin_port = htons(9000);
    ser.sin_addr.s_addr = htonl(INADDR_ANY);

    status = connect(gniazdo, (struct sockaddr*) &ser, sizeof(ser));
    if (status < 0) {
        printf("Błąd connect\n");
        return 0;
    }

    printf("Podaj tekst: ");
    fgets(buf, sizeof(buf), stdin);
    status = write(gniazdo, buf, strlen(buf));
    status = read(gniazdo, buf, sizeof(buf) - 1);
    buf[status] = '\0';
    printf("Otrzymałem: %s\n", buf);

    close(gniazdo);
    printf("KONIEC DZIAŁANIA KLIENTA\n");
    return 0;
}
