#include "common.h"


struct client clients[100];
int current_id = 0;
int listen_socket;
int epoll_fd;
time_t last_ping_time;

void close_client(struct client* client) {
    client->active = 0;
    shutdown(client->socket_desc, SHUT_RDWR);
    close(client->socket_desc);
}

char* format_client(const struct client *c) {
    char *buffer = malloc(64);
    snprintf(buffer, 64, "ID: %d; NAME: %s;", c->id, c->name);
    return buffer;
}


char* format_all_clients() {
    size_t total_size = 64 * current_id;
    char *result = malloc(total_size);

    result[0] = '\0';

    for (int i = 0; i < current_id; i++) {
        if (!clients[i].active) continue;

        char *line = format_client(&clients[i]);
        if (line == NULL) continue;

        strncat(result, line, total_size - strlen(result) - 1);
        strncat(result, "\n", total_size - strlen(result) - 1);
        free(line);
    }

    return result;
}

void send_ping_to_all() {
    for (int i = 0; i < current_id; i++) {
        if (!clients[i].active) continue;
        struct message ping_msg;
        ping_msg.type = PING;
        ping_msg.sender_id = -1;
        ping_msg.receiver_id = clients[i].id;
        if (write(clients[i].socket_desc, &ping_msg, sizeof(ping_msg)) < 0) {
            printf("Nie udało się wysłać PING do klienta");
            close_client(i);
        }
    }
    printf("Wysłano PING do wszystkich klientów.\n");
}



void exit_handler() {
    shutdown(listen_socket, SHUT_RDWR);
    close(listen_socket);
    for (int i = 0; i < current_id; i++) {
        if (clients[i].active) {
            close_client(&clients[i]);
        }
    }
    close(epoll_fd);
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

    epoll_fd = epoll_create1(0);
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = listen_socket;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_socket, &ev);

    struct message incoming, outgoing;
    struct epoll_event events[100];
    while (1) {
        time_t now = time(NULL);
        if (now - last_ping_time >= PING_TIMEOUT) {
            send_ping_to_all();
            last_ping_time = now;
        }
        
        outgoing.type = MES;

        int events_ready = epoll_wait(epoll_fd, events, 100, -1);

        memset(&incoming, 0, sizeof(incoming));

        struct tm* local = localtime(&now);

        for (int i = 0; i < events_ready; i++) {
            int fd = events[i].data.fd;
            int event = events[i].events;

            if (fd == listen_socket) {
                int incoming_socket = accept(listen_socket, NULL, NULL);

                read(incoming_socket, &incoming, sizeof(incoming));

                struct client new_client;
                new_client.id = current_id;
                strcpy(new_client.name, incoming.content);
                new_client.socket_desc = incoming_socket;
                new_client.active = 1;
                new_client.last_ping = time(NULL);
                new_client.pinged_iter_ago = 0;
                clients[current_id] = new_client;

                struct epoll_event client_event;
                client_event.events = EPOLLIN | EPOLLET; // Edge triggered, jednokrotnie emitowane powiadomienie
                client_event.data.fd = incoming_socket;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, incoming_socket, &client_event);

                outgoing.receiver_id = current_id;
                printf("Zarejestrowałem nowego klienta o imieniu '%s' i ID=%d\n", incoming.content, current_id);
                sprintf(outgoing.content, "Klient został zapisany z indeksem %d\n", current_id++);
                write(incoming_socket, &outgoing, sizeof(outgoing));
            } else {
                read(fd, &incoming, sizeof(incoming));
                switch (incoming.type) {
                    case LIST:
                        outgoing.type = LIST;
                        strcpy(outgoing.content, format_all_clients());
                        write(fd, &outgoing, sizeof(outgoing));
                        printf("Wysłano listę klientów klientowi %d\n", incoming.sender_id);
                        break;
        
                    case TO_ALL:
                        outgoing.type = MES;
                        outgoing.sender_id = incoming.sender_id;
                        //incoming.content[strcspn(incoming.content, "\n")] = '\0';
                        snprintf(outgoing.content, sizeof(outgoing.content), "%d.%d.%dr. %s", local->tm_mday, local->tm_mon + 1, local->tm_year + 1900, incoming.content);
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
                        snprintf(outgoing.content, sizeof(outgoing.content), "%d.%d.%dr. %s", local->tm_mday, local->tm_mon + 1, local->tm_year + 1900, incoming.content);
                        if (!clients[incoming.receiver_id].active || incoming.receiver_id >= current_id) {
                            printf("Probowano wysłać wiadomość do nieistniejącego klienta\n");
                            continue;
                        }
                        write(clients[incoming.receiver_id].socket_desc, &outgoing, sizeof(outgoing));
                        printf("Przesłano wiadomość '%s' klientowi %d.\n", outgoing.content, incoming.receiver_id);
                        break;
                    
                    case STOP:
                        close_client(&clients[incoming.sender_id]);
                        printf("Usunięto klienta %d.\n", incoming.sender_id);
                        break;

                    case PONG:
                        clients[incoming.sender_id].last_ping = time(NULL);
                        clients[incoming.sender_id].pinged_iter_ago = 0;
                        printf("Dostałem odpowiedź na ping od klienta o ID=%d\n", incoming.sender_id);
                        break;
        
                    default:
                        printf("Otrzymano niezrozumiałe polecenie od klienta nr %d.\n", incoming.sender_id);
                    }
            }
        }
        now = time(NULL);
        for (int i = 0; i < current_id; i++) {
            if (!clients[i].active || clients[i].last_ping == 0 || clients[i].pinged_iter_ago < 2) {
                clients[i].pinged_iter_ago += 1;
                continue;
            } 
            if (now - clients[i].last_ping > PING_TIMEOUT) {
                printf("Klient ID=%d nie odpowiedział na PING => usuwam.\n", clients[i].id);
                close_client(&clients[i]);
            }
            clients[i].pinged_iter_ago += 1;
        }
    }
}
