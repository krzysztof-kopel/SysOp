#include "common.h"

int my_socket;
int my_id;
pid_t pid; // Pomaga mi określić, czy to proces czytający czy wysyłający
struct sockaddr_in server_address;

void handler() {
    if (pid != 0) {
        struct message mess_end;
        mess_end.type = STOP;
        mess_end.sender_id = my_id;
        sendto(my_socket, &mess_end, sizeof(mess_end), 0, (struct sockaddr*) &server_address, sizeof(server_address));

        close(my_socket);
        printf("Klient zamknięty.\n");
    }
    exit(0);
}

int main(int argc, char** argv) {
    signal(SIGINT, handler);
    if (argc != 4) {
        printf("Nieprawidłowa liczba argumentów.\n");
        return -1;
    }

    char* name = argv[1];
    char* ipv4_server_address = argv[2];
    int port = atoi(argv[3]);

    my_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (my_socket == -1) {
        printf("Błąd socket\n");
        return -1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(ipv4_server_address);

    struct message welcome;
    welcome.type = NEW;
    strcpy(welcome.content, name);
    sendto(my_socket, &welcome, sizeof(welcome), 0, (struct sockaddr*) &server_address, sizeof(server_address));

    struct message server_confirmation;
    recvfrom(my_socket, &server_confirmation, sizeof(server_confirmation), 0, NULL, NULL);
    my_id = server_confirmation.receiver_id;
    printf("Zostało mi przyznane ID %d.\n", my_id);

    char buffer[100];
    struct message mess;
    mess.sender_id = my_id;

    pid = fork();


    if (pid == 0) {
        while (1) {
            struct message incoming, ping;
            
            recvfrom(my_socket, &incoming, sizeof(incoming), 0, NULL, NULL);

            if (incoming.type == LIST) {
                printf("Lista klientów:\n");
                printf("%s\n", incoming.content);
            } else if (incoming.type == MES) {
                printf("Wiadomość od klienta %d o treści: %s\n", incoming.sender_id, incoming.content);
            } else if (incoming.type == PING) {
                printf("Otrzymałem ping.\n");
                ping.type = PONG;
                ping.sender_id = my_id;
                sendto(my_socket, &ping, sizeof(ping), 0, (struct sockaddr*) &server_address, sizeof(server_address));
            } else {
                printf("Otrzymałem niezrozumiałą wiadomość.\n");
            }
        }
    } else {
        while (pid != 0) {
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\r\n")] = '\0';
            buffer[100] = '\0';
            char* first_word = strtok(buffer, " ");
            char* rest = strtok(NULL, "");
    
            if (strcmp(first_word, "LIST") == 0) {
                mess.type = LIST;
                mess.sender_id = my_id;
                
                sendto(my_socket, &mess, sizeof(mess), 0, (struct sockaddr*) &server_address, sizeof(server_address));
    
            } else if (strcmp(first_word, "2ALL") == 0) {
                mess.type = TO_ALL;
                
                rest[strcspn(rest, "\r\n")] = '\0';
                strncpy(mess.content, rest, 100);
    
                sendto(my_socket, &mess, sizeof(mess), 0, (struct sockaddr*) &server_address, sizeof(server_address));
            } else if (strcmp(first_word, "2ONE") == 0) {
                mess.type = TO_ONE;
                mess.receiver_id = atoi(strtok(rest, " "));

                rest[strcspn(rest, "\r\n")] = '\0';
                strncpy(mess.content, strtok(NULL, ""), 100);
    
                sendto(my_socket, &mess, sizeof(mess), 0, (struct sockaddr*) &server_address, sizeof(server_address));
            } else if (strcmp(first_word, "STOP") == 0) {
                kill(pid, SIGINT);
                raise(SIGINT);
            } else {
                printf("Niezrozumiałe polecenie.\n");
            }
        }
    }
}
