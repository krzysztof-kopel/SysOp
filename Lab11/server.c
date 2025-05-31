#include "common.h"


struct client clients[100];
int current_id = 0;
int listen_socket;

void close_client(struct client* client) {
    client->active = 0;
    close(client->socket_desc);
}

void exit_handler() {
    close(listen_socket);
    for (int i = 0; i < current_id; i++) {
        if (clients[i].active) {
            close_client(&clients[i]);
        }
    }
    printf("Wszystkie zasoby zwolnione.\n");
    exit(0);
}

int main(int argc, char** argv) {
    signal(SIGINT, exit_handler);
    signal(SIGCHLD, SIG_IGN);
    if (argc != 2) {
        printf("Nieprawidłowa liczba arugmentów\n");
        return -1;
    }

    int port = atoi(argv[1]);

    listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket == -1) {
        printf("Błąd socket\n");
        return -1;
    }

    struct sockaddr_in lis_address;
    lis_address.sin_family = AF_INET;
    lis_address.sin_port = htons(port);
    lis_address.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("Adres serwera: %s\n", inet_ntoa(lis_address.sin_addr));

    if (bind(listen_socket, (struct sockaddr*) &lis_address, sizeof(lis_address))) {
        printf("Błąd bind\n");
        return -1;
    }

    if (listen(listen_socket, 10) == -1) {
        printf("Błąd listen\n");
        return -1;
    }

    struct message incoming, outgoing;
    while (1) {
        outgoing.type = MES;
        int incoming_socket = accept(listen_socket, NULL, NULL);
        if (read(incoming_socket, &incoming, sizeof(incoming)) <= 0) {
            exit(0);
        }
        time_t now = time(NULL);
        struct tm* local = localtime(&now);
        switch (incoming.type) {
            case NEW:
                struct client new_client;
                new_client.id = current_id;
                strcpy(new_client.name, incoming.content);
                new_client.socket_desc = incoming_socket;
                new_client.active = 1;
                clients[current_id] = new_client;

                outgoing.receiver_id = current_id;
                sprintf(outgoing.content, "Klient został zapisany z indeksem %d\n", current_id++);
                write(incoming_socket, &outgoing, sizeof(outgoing));
                break;

            case LIST:
                struct client_list_message outgoing_list;
                memcpy(&outgoing_list.clients, clients, sizeof(outgoing_list.clients));
                write(incoming_socket, &outgoing_list, sizeof(outgoing_list));
                printf("Wysłano listę klientów klientowi %d\n", incoming.sender_id);
                break;

            case TO_ALL:
                outgoing.sender_id = incoming.sender_id;
                sprintf(outgoing.content, "%02d:%02d:%02d %.88s\n", local->tm_hour, local->tm_min, local->tm_sec, incoming.content);
                for (int i = 0; i < current_id; i++) {
                    if (clients[i].id == incoming.sender_id || !clients[i].active) {
                        continue;
                    }
                    write(clients[i].socket_desc, &outgoing, sizeof(outgoing));
                }
                printf("Rozesłano wiadomość '%s' wszystkim klientom.\n", outgoing.content);
                break;
            
            case TO_ONE:
                outgoing.sender_id = incoming.sender_id;
                sprintf(outgoing.content, "%02d:%02d:%02d %.88s\n", local->tm_hour, local->tm_min, local->tm_sec, incoming.content);
                if (!clients[incoming.receiver_id].active) {
                    printf("Probowano wysłać wiadomość do nieistniejącego klienta\n");
                }
                write(clients[incoming.receiver_id].socket_desc, &outgoing, sizeof(outgoing));
                printf("Przesłano wiadomość '%s' klientowi %d.\n", outgoing.content, incoming.receiver_id);
                break;
            
            case STOP:
                close_client(&clients[incoming.sender_id]);
                printf("Usunięto klienta %d.\n", incoming.sender_id);
                break;

            case ALIVE:
                outgoing.type = ALIVE;
                for (int i = 0; i < current_id; i++) {
                    if (!clients[i].active) {
                        continue;
                    }
                    clients[i].active = 0;
                    write(incoming_socket, &outgoing, sizeof(outgoing));
                }
                printf("Zapingowano wszystkich klientów.\n");
                break;
            
            case PING:
                clients[incoming.sender_id].active = 1;
                break;

            default:
                printf("Otrzymano niezrozumiałe polecenie od klienta nr %d.\n", incoming.sender_id);
        }
        exit(0);
    }
}
