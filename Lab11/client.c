#include "common.h"

int my_socket;
int my_id;
pid_t pid; // Pomaga mi określić, czy to proces czytający czy wysyłający

void handler() {
    if (pid != 0) {
        struct message mess_end;
        mess_end.type = STOP;
        mess_end.sender_id = my_id;
        write(my_socket, &mess_end, sizeof(mess_end));

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
    char* ipv4_address = argv[2];
    int port = atoi(argv[3]);

    my_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (my_socket == -1) {
        printf("Błąd socket\n");
        return -1;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = inet_addr(ipv4_address);

    if (connect(my_socket, (struct sockaddr*) &address, sizeof(address)) < 0) {
        printf("Błąd connect.\n");
        return -1;
    }

    struct message welcome;
    welcome.type = NEW;
    strcpy(welcome.content, name);
    write(my_socket, &welcome, sizeof(welcome));

    read(my_socket, &welcome, sizeof(welcome));
    my_id = welcome.receiver_id;
    printf("Zostało mi przyznane ID %d.\n", my_id);

    char buffer[100];
    struct message mess;
    mess.sender_id = my_id;

    pid = fork();


    if (pid == 0) {
        while (1) {
            struct message incoming;
            
            read(my_socket, &incoming, sizeof(incoming));
    
            if (incoming.type == LIST) {
                printf("Lista klientów:\n");
                printf("%s\n", incoming.content);
            } else if (incoming.type == MES) {
                printf("Wiadomość od klienta %d o treści: %s\n", incoming.sender_id, incoming.content);
            } else {
                printf("Otrzymałem niezrozumiałą wiadomość.\n");
            }
        }
    } else {
        while (pid != 0) {
            fgets(buffer, sizeof(buffer), stdin);
            char* first_word = strtok(buffer, " ");
            char* rest = strtok(NULL, "");
    
            if (strcmp(first_word, "LIST\n") == 0) {
                mess.type = LIST;
                mess.sender_id = my_id;
                
                write(my_socket, &mess, sizeof(mess));
                printf("Wysłano LIST\n");
    
            } else if (strcmp(first_word, "2ALL") == 0) {
                mess.type = TO_ALL;
                
                strncpy(mess.content, rest, 100);
    
                write(my_socket, &mess, sizeof(mess));
            } else if (strcmp(first_word, "2ONE") == 0) {
                mess.type = TO_ONE;
                mess.receiver_id = atoi(strtok(rest, " "));
                strncpy(mess.content, strtok(NULL, ""), 100);
    
                write(my_socket, &mess, sizeof(mess));
            } else if (strcmp(first_word, "STOP") == 0) {
                kill(pid, SIGINT);
                raise(SIGINT);
            } else {
                printf("Niezrozumiałe polecenie.\n");
            }
        }
    }
}
