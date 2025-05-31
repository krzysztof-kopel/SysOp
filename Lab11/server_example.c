#include "common.h"

int main(void) {
    int status, gniazdo, dlugosc, nr = 0, end = 1, gniazdo2;
    struct sockaddr_in ser, cli;
    char buf[200];

    // Tworzymy gniazdo
    gniazdo = socket(AF_INET, SOCK_STREAM, 0);

    if (gniazdo == -1) {
        printf("Błąd socket\n");
        return 0;
    }

    // Tworzymy i konfigurujemy strukturę adresu
    memset(&ser, 0, sizeof(ser));
    ser.sin_family = AF_INET;
    ser.sin_port = htons(9000);
    ser.sin_addr.s_addr = htons(INADDR_ANY);
    // Ustawiamy adres
    status = bind(gniazdo, (struct sockaddr*) &ser, sizeof(ser));
    if (status == -1) {
        printf("Błąd bind\n");
        return 0;
    }

    // Zaczynamy nasłuchiwać (maks. 10 połączeń w jednym momencie)
    status = listen(gniazdo, 10);
    if (status == -1) {
        printf("Błąd listen\n");
        return 0;
    }

    while (end) {
        dlugosc = sizeof(cli);
        gniazdo2 = accept(gniazdo, (struct sockaddr*) &cli, (socklen_t *) &dlugosc);
        if (gniazdo2 == -1) {
            printf("Błąd accept\n");
            return 0;
        }
        read(gniazdo2, buf, sizeof(buf));
        if (buf[0] == 'Q') {
            sprintf(buf, "ZGODNA, SERWER KONCZY PRACE");
            end = 0;
        } else if (buf[0] == 'N') {
            sprintf(buf, "Jestes klientem nr %d", nr++);
        } else {
            sprintf(buf, "Nie rozumiem pytania");
        }
        write(gniazdo2, buf, strlen(buf));
        close(gniazdo2);
    }
    close(gniazdo);
    printf("KONIEC DZIAŁANIA SERWERA\n");
    return 0;
}