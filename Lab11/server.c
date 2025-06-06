#include "common.h"

struct client clients[100];
int current_id = 0;
int listen_socket;
char all_clients_list[100];
pthread_t worker_threads[100];
pthread_mutex_t clients_access_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t one_formatter_mutex = PTHREAD_MUTEX_INITIALIZER;

void close_client(struct client* client) {
    client->active = 0;
    close(client->socket_desc);
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

void* handle_existing_client(void* data) {
    struct client* my_client = (struct client*) data;
    struct message incoming, outgoing;
    while (1) {
        if (read(my_client->socket_desc, &incoming, sizeof(incoming)) <= 0) {
            pthread_mutex_lock(&clients_access_mutex);
            close_client(my_client);
            pthread_mutex_unlock(&clients_access_mutex);
            return (void*)my_client;
        }
        time_t now = time(NULL);
        struct tm* local = localtime(&now);
        
        switch (incoming.type) {
            case LIST:
                outgoing.type = LIST;
                pthread_mutex_lock(&clients_access_mutex);
                pthread_mutex_lock(&one_formatter_mutex);
                format_all_clients();
                strcpy(outgoing.content, all_clients_list);
                outgoing.content[sizeof(outgoing.content) - 1] = '\0';
                write(my_client->socket_desc, &outgoing, sizeof(outgoing));
                printf("Wysłano listę klientów klientowi %d\n", incoming.sender_id);
                pthread_mutex_unlock(&one_formatter_mutex);
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
                    write(clients[i].socket_desc, &outgoing, sizeof(outgoing));
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
                write(clients[incoming.receiver_id].socket_desc, &outgoing, sizeof(outgoing));
                printf("Przesłano wiadomość '%s' klientowi %d.\n", outgoing.content, incoming.receiver_id);
                break;
            
            case STOP:
                pthread_mutex_lock(&clients_access_mutex);
                close_client(&clients[incoming.sender_id]);
                printf("Usunięto klienta %d.\n", incoming.sender_id);
                break;

            default:
                printf("Otrzymano niezrozumiałe polecenie od klienta nr %d.\n", incoming.sender_id);
        }
        pthread_mutex_unlock(&clients_access_mutex);
    }
}


void exit_handler() {
    for (int i = 0; i < current_id; i++) {
        pthread_cancel(worker_threads[i]);
    }
    shutdown(listen_socket, SHUT_RDWR);
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

    struct message first_contact, first_response;
    while (1) {
        first_response.type = MES;

        memset(&first_contact, 0, sizeof(first_contact));

        int incoming_socket = accept(listen_socket, NULL, NULL);

        read(incoming_socket, &first_contact, sizeof(first_contact));

        pthread_mutex_lock(&clients_access_mutex);
        struct client new_client;
        new_client.id = current_id;
        strcpy(new_client.name, first_contact.content);
        new_client.socket_desc = incoming_socket;
        new_client.active = 1;
        clients[current_id] = new_client;
        pthread_mutex_unlock(&clients_access_mutex);

        first_response.receiver_id = current_id;
        printf("Zarejestrowałem nowego klienta o imieniu '%s' i ID=%d\n", first_contact.content, current_id);
        sprintf(first_response.content, "Klient został zapisany z indeksem %d\n", current_id);
        write(incoming_socket, &first_response, sizeof(first_response));

        pthread_create(&worker_threads[current_id], 0, &handle_existing_client, &clients[current_id]);
        current_id++;
    }

    return 0;
}
