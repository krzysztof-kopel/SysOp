#include "common.h"

struct client clients[100];
int current_id = 0;
int server_socket;
char all_clients_list[100];
pthread_mutex_t clients_access_mutex = PTHREAD_MUTEX_INITIALIZER;

void close_client(struct client* client) {
    client->active = 0;
}

char* format_client(const struct client *c) {
    char *buffer = malloc(64);
    snprintf(buffer, 64, "ID: %d; NAME: %s;", c->id, c->name);
    return buffer;
}


void format_all_clients() {
    size_t total_size = 64 * current_id;
    char *result = malloc(total_size);
    memset(result, 0, sizeof(result));

    result[0] = '\0';

    for (int i = 0; i < current_id; i++) {
        if (!clients[i].active) continue;

        char *line = format_client(&clients[i]);
        if (line == NULL) continue;

        strncat(result, line, total_size - strlen(result) - 1);
        strncat(result, "\n", total_size - strlen(result) - 1);
        free(line);
    }
    strcpy(all_clients_list, result);
}

void* pinger(void*) {
    struct message ping_mess;
    ping_mess.type = PING;
    while (1) {
        for (int i = 0; i < current_id; i++) {
            pthread_mutex_lock(&clients_access_mutex);
            if (!clients[i].active) {
                pthread_mutex_unlock(&clients_access_mutex);
                continue;
            }

            if (time(NULL) - clients[i].last_ping > PING_TIMEOUT) {
                printf("Zamykam klienta o ID=%d, gdyż ten zbyt długo nie odpowiadał.\n", clients[i].id);
                close_client(&clients[i]);
            } else {
                sendto(server_socket, &ping_mess, sizeof(ping_mess), 0, (struct sockaddr*) &clients[i].address, sizeof(clients[i].address));
            }

            pthread_mutex_unlock(&clients_access_mutex);
        }
        sleep(10);
    }
}

void exit_handler() {
    shutdown(server_socket, SHUT_RDWR);
    close(server_socket);
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
    if (argc != 2) {
        printf("Nieprawidłowa liczba arugmentów\n");
        return -1;
    }

    int port = atoi(argv[1]);

    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket == -1) {
        printf("Błąd socket\n");
        return -1;
    }

    struct sockaddr_in lis_address;
    lis_address.sin_family = AF_INET;
    lis_address.sin_port = htons(port);
    lis_address.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("Adres serwera: %s\n", inet_ntoa(lis_address.sin_addr));

    if (bind(server_socket, (struct sockaddr*) &lis_address, sizeof(lis_address))) {
        printf("Błąd bind\n");
        return -1;
    }

    pthread_t ping_thread;
    pthread_create(&ping_thread, 0, &pinger, NULL);

    struct message incoming, outgoing;
    struct sockaddr_in incoming_address;
    while (1) {
        memset(&incoming, 0, sizeof(incoming));
        socklen_t address_size = sizeof(incoming_address);
        recvfrom(server_socket, &incoming, sizeof(incoming), 0, (struct sockaddr*) &incoming_address, &address_size);

        time_t now = time(NULL);
        struct tm* local = localtime(&now);

        switch(incoming.type) {
            case NEW:
                outgoing.type = MES;

                pthread_mutex_lock(&clients_access_mutex);
                struct client new_client;
                new_client.id = current_id;
                strcpy(new_client.name, incoming.content);
                memcpy(&new_client.address, &incoming_address, sizeof(struct sockaddr_in));
                new_client.active = 1;
                new_client.last_ping = time(NULL);
                clients[current_id] = new_client;
                pthread_mutex_unlock(&clients_access_mutex);

                outgoing.receiver_id = current_id;
                printf("Zarejestrowałem nowego klienta o imieniu '%s' i ID=%d\n", incoming.content, current_id);
                sprintf(outgoing.content, "Klient został zapisany z indeksem %d\n", current_id);
                sendto(server_socket, &outgoing, sizeof(outgoing), 0, (struct sockaddr*) &incoming_address, sizeof(incoming_address));

                current_id++;
                break;

            case LIST:
                outgoing.type = LIST;
                pthread_mutex_lock(&clients_access_mutex);
                format_all_clients();
                strcpy(outgoing.content, all_clients_list);
                outgoing.content[sizeof(outgoing.content) - 1] = '\0';
                sendto(server_socket, &outgoing, sizeof(outgoing), 0, (struct sockaddr*) &incoming_address, sizeof(incoming_address));
                printf("Wysłano listę klientów klientowi %d\n", incoming.sender_id);
                pthread_mutex_unlock(&clients_access_mutex);
                break;

            case TO_ALL:
                outgoing.type = MES;
                outgoing.sender_id = incoming.sender_id;
                //incoming.content[strcspn(incoming.content, "\n")] = '\0';
                snprintf(outgoing.content, sizeof(outgoing.content), "%d.%d.%dr. %.60s", local->tm_mday, local->tm_mon + 1, local->tm_year + 1900, incoming.content);
                pthread_mutex_lock(&clients_access_mutex);
                for (int i = 0; i < current_id; i++) {
                    if (clients[i].id == incoming.sender_id || !clients[i].active) {
                        continue;
                    }
                    sendto(server_socket, &outgoing, sizeof(outgoing), 0, (struct sockaddr*) &clients[i].address, sizeof(clients[i].address));
                }
                printf("Rozesłano wiadomość '%s' wszystkim klientom.\n", outgoing.content);
                break;
            
            case TO_ONE:
                outgoing.type = MES;
                outgoing.sender_id = incoming.sender_id;
                //incoming.content[strcspn(incoming.content, "\n")] = '\0';
                snprintf(outgoing.content, sizeof(outgoing.content), "%d.%d.%dr. %.60s", local->tm_mday, local->tm_mon + 1, local->tm_year + 1900, incoming.content);
                pthread_mutex_lock(&clients_access_mutex);
                if (!clients[incoming.receiver_id].active || incoming.receiver_id >= current_id) {
                    printf("Probowano wysłać wiadomość do nieistniejącego klienta\n");
                    continue;
                }
                sendto(server_socket, &outgoing, sizeof(outgoing), 0, (struct sockaddr*) &clients[incoming.receiver_id].address, 
                        sizeof(clients[incoming.receiver_id].address));
                printf("Przesłano wiadomość '%s' klientowi %d.\n", outgoing.content, incoming.receiver_id);
                break;
            
            case STOP:
                pthread_mutex_lock(&clients_access_mutex);
                close_client(&clients[incoming.sender_id]);
                printf("Usunięto klienta %d.\n", incoming.sender_id);
                break;

            case PONG:
                pthread_mutex_lock(&clients_access_mutex);
                clients[incoming.sender_id].last_ping = time(NULL);
                break;

            default:
                printf("Otrzymano niezrozumiałe polecenie od klienta nr %d.\n", incoming.sender_id);
        }
        pthread_mutex_unlock(&clients_access_mutex);       
    }
    return 0;
}
