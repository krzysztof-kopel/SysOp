#include "common.h"

int main(void) {
    int status, gniazdo, dlugosc, nr = 0, end = 1, gniazdo2, l_bajtow;
    struct sockaddr_in cli, ser;
    char buf[200];

    gniazdo = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&ser, 0, sizeof(ser));
    ser.sin_family = AF_INET;
    ser.sin_port = htons(9001);
    ser.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(gniazdo, (struct sockaddr*) &ser, sizeof(ser));

    while (1) {
        dlugosc = sizeof(cli);
        l_bajtow = recvfrom(gniazdo, &buf, sizeof(buf), 0, (struct sockaddr*) cli, )
    }
}