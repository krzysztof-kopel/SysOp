// client.c
#include "common.h"

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int my_sock = -1;
static int my_id = -1;
static char my_name[50];

/* Gdy klient naciśnie Ctrl+C – wysyłamy STOP i wychodzimy */
void sigint_handler(int signo) {
    (void)signo;
    if (my_id >= 0) {
        struct message msg;
        msg.type = STOP;
        msg.sender_id = my_id;
        msg.receiver_id = -1;
        strcpy(msg.content, "STOP");
        write(my_sock, &msg, sizeof(msg));
    }
    close(my_sock);
    printf("\n[CLIENT] Rozłączam się.\n");
    exit(0);
}

/* Proces potomny: czyta z serwera i wypisuje komunikaty na stdout */
void reader_process() {
    while (1) {
        struct message inc;
        ssize_t r = read(my_sock, &inc, sizeof(inc));
        if (r <= 0) {
            printf("[CLIENT] Serwer zakończył połączenie.\n");
            close(my_sock);
            exit(0);
        }
        if (inc.type == NEW) {
            // To jest potwierdzenie rejestracji; inc.sender_id to przydzielone ID
            my_id = inc.sender_id;
            printf("[CLIENT] %s\n", inc.content);
            printf("[CLIENT] Twój ID to %d\n", my_id);
        }
        else if (inc.type == MES) {
            // To jest wiadomość od serwera (np. wynik LIST lub komunikat od innego klienta)
            printf("%s\n", inc.content);
        }
        else {
            // Dla bezpieczeństwa: jeśli serwer odsyła coś innego, też to wypisujemy
            printf("%s\n", inc.content);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Użycie: %s <nazwa_klienta> <adres IPv4> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    strncpy(my_name, argv[1], sizeof(my_name) - 1);
    my_name[sizeof(my_name) - 1] = '\0';
    char *server_ip = argv[2];
    int server_port = atoi(argv[3]);

    // 1) Tworzymy gniazdo i łączymy się z serwerem
    my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock < 0) {
        perror("[CLIENT] socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("[CLIENT] inet_pton");
        exit(EXIT_FAILURE);
    }
    if (connect(my_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("[CLIENT] connect");
        exit(EXIT_FAILURE);
    }

    // 2) Obsługa Ctrl+C
    signal(SIGINT, sigint_handler);

    // 3) Wysyłamy komunikat NEW z naszą nazwą
    struct message msg;
    msg.type = NEW;
    msg.sender_id = -1;   // serwer nam przydzieli ID
    msg.receiver_id = -1;
    strncpy(msg.content, my_name, sizeof(msg.content) - 1);
    msg.content[sizeof(msg.content) - 1] = '\0';
    if (write(my_sock, &msg, sizeof(msg)) < 0) {
        perror("[CLIENT] write(NEW)");
        exit(EXIT_FAILURE);
    }

    // 4) Fork: proces potomny czyta z serwera; proces rodzicielski wysyła komendy
    pid_t pid = fork();
    if (pid < 0) {
        perror("[CLIENT] fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        // Proces potomny: czyta i wypisuje to, co serwer wysyła
        reader_process();
        // nigdy stąd nie wracamy
    } else {
        // Proces rodzicielski: czyta linie z stdin i wysyła komendy do serwera
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            // Usuń kończące \n lub \r\n
            buffer[strcspn(buffer, "\r\n")] = '\0';

            if (strncmp(buffer, "LIST", 4) == 0 && (buffer[4] == '\0' || buffer[4] == ' ')) {
                struct message m2;
                m2.type = LIST;
                m2.sender_id = my_id;
                m2.receiver_id = -1;
                m2.content[0] = '\0';
                write(my_sock, &m2, sizeof(m2));
            }
            else if (strncmp(buffer, "2ALL ", 5) == 0) {
                struct message m2;
                m2.type = TO_ALL;
                m2.sender_id = my_id;
                m2.receiver_id = -1;
                strncpy(m2.content, buffer + 5, sizeof(m2.content) - 1);
                m2.content[sizeof(m2.content) - 1] = '\0';
                write(my_sock, &m2, sizeof(m2));
            }
            else if (strncmp(buffer, "2ONE ", 5) == 0) {
                // Format: 2ONE <id> <tekst>
                struct message m2;
                m2.type = TO_ONE;
                m2.sender_id = my_id;
                int target;
                char tekst[100];
                if (sscanf(buffer + 5, "%d %[^\n]", &target, tekst) >= 2) {
                    m2.receiver_id = target;
                    strncpy(m2.content, tekst, sizeof(m2.content) - 1);
                    m2.content[sizeof(m2.content) - 1] = '\0';
                    write(my_sock, &m2, sizeof(m2));
                } else {
                    printf("[CLIENT] Błędna komenda 2ONE. Użycie: 2ONE <id> <wiadomość>\n");
                }
            }
            else if (strncmp(buffer, "STOP", 4) == 0 && (buffer[4] == '\0' || buffer[4] == ' ')) {
                struct message m2;
                m2.type = STOP;
                m2.sender_id = my_id;
                m2.receiver_id = -1;
                m2.content[0] = '\0';
                write(my_sock, &m2, sizeof(m2));
                break;  // wychodzimy z pętli – Ctrl+C i tak zostanie przejęty przez handler
            }
            else {
                printf("[CLIENT] Nieznana komenda: %s\n", buffer);
                printf("         Dostępne: LIST, 2ALL <tekst>, 2ONE <id> <tekst>, STOP\n");
            }
        }
        // Jeśli użytkownik nacisnął Ctrl+D (EOF), też wykonujemy wylogowanie
        sigint_handler(0);
    }

    return 0;
}
