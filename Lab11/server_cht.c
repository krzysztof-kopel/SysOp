// server.c
#include "common.h"
#include <sys/select.h>
#include <sys/socket.h>

#define MAX_CLIENTS 100

static struct client clients[MAX_CLIENTS];
static int current_id = 0;
static int listen_fd = -1;

/* Usuwa klienta o indeksie idx: zamyka socket i oznacza jego slot jako nieaktywny */
void remove_client(int idx) {
    if (clients[idx].active) {
        close(clients[idx].socket_desc);
        clients[idx].active = 0;
        printf("[SERVER] Klient ID=%d (%s) odłączony.\n",
               clients[idx].id, clients[idx].name);
    }
}

/* Formatuje informację o jednym kliencie do postaci "ID: <id>, NAME: <nazwa>\n" */
void format_client_info(const struct client *c, char *out_buf) {
    snprintf(out_buf, 64, "ID: %d, NAME: %s\n", c->id, c->name);
}

/* Wysyła do klienta o indeksie idx komunikat typu MES z treścią msg_buf */
void send_message_to_client(int idx, const char *msg_buf, int sender_id) {
    struct message out;
    out.type = MES;
    out.sender_id = sender_id;
    out.receiver_id = clients[idx].id;
    /* common.h definiuje content jako char content[100]; */
    strncpy(out.content, msg_buf, 100 - 1);
    out.content[100 - 1] = '\0';

    if (write(clients[idx].socket_desc, &out, sizeof(out)) < 0) {
        perror("[SERVER] Błąd write() do klienta");
        remove_client(idx);
    }
}

/* Obsługa Ctrl+C – zamyka listen_fd i wszystkie gniazda klientów, a potem kończy program */
void handle_sigint(int sig) {
    (void)sig;
    printf("\n[SERVER] Wyłączanie serwera, zamykam wszystkie połączenia...\n");
    if (listen_fd >= 0) close(listen_fd);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            close(clients[i].socket_desc);
        }
    }
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Użycie: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    signal(SIGINT, handle_sigint);

    int port = atoi(argv[1]);

    // 1) Tworzymy gniazdo nasłuchujące
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    // Ustawiamy SO_REUSEADDR, aby po restarcie serwera port nie był zablokowany
    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    if (listen(listen_fd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("[SERVER] Nasłuchiwanie na porcie %d...\n", port);

    // 2) Inicjalizujemy tablicę klientów
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].active = 0;
    }

    // 3) Główna pętla z select()
    fd_set read_fds;
    int max_fd = listen_fd;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(listen_fd, &read_fds);
        // Dodajemy każde aktywne gniazdo klienta do read_fds
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].active) {
                FD_SET(clients[i].socket_desc, &read_fds);
                if (clients[i].socket_desc > max_fd) {
                    max_fd = clients[i].socket_desc;
                }
            }
        }

        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("[SERVER] select");
            continue;
        }

        // 4) Jeżeli listen_fd jest gotowy → nowe połączenie
        if (FD_ISSET(listen_fd, &read_fds)) {
            struct sockaddr_in cli_addr;
            socklen_t cli_len = sizeof(cli_addr);
            int new_fd = accept(listen_fd, (struct sockaddr *)&cli_addr, &cli_len);
            if (new_fd < 0) {
                perror("[SERVER] accept");
            } else {
                // Odczytujemy od razu strukturę NEW
                struct message inc;
                memset(&inc, 0, sizeof(inc));
                ssize_t r = read(new_fd, &inc, sizeof(inc));
                if (r <= 0 || inc.type != NEW) {
                    // Klient nie wysłał poprawnej struktury NEW albo rozłączył się
                    close(new_fd);
                } else {
                    // Rejestrujemy nowego klienta
                    int idx;
                    for (idx = 0; idx < MAX_CLIENTS; idx++) {
                        if (!clients[idx].active) {
                            clients[idx].active = 1;
                            clients[idx].id = current_id++;
                            clients[idx].socket_desc = new_fd;
                            /* common.h: name ma długość 50 */
                            strncpy(clients[idx].name, inc.content, 50 - 1);
                            clients[idx].name[50 - 1] = '\0';
                            break;
                        }
                    }
                    if (idx == MAX_CLIENTS) {
                        // Brak wolnego miejsca
                        printf("[SERVER] Brak miejsca, odrzucono klienta %s\n", inc.content);
                        close(new_fd);
                    } else {
                        // Wysyłamy potwierdzenie rejestracji (typ=NEW, sender_id=ID klienta)
                        struct message out;
                        out.type = NEW;
                        out.sender_id = clients[idx].id;
                        out.receiver_id = -1;
                        snprintf(out.content, 100,
                                 "Witaj %s! Twój ID to %d.",
                                 clients[idx].name, clients[idx].id);
                        write(new_fd, &out, sizeof(out));
                        printf("[SERVER] Nowy klient: ID=%d, NAME=%s\n",
                               clients[idx].id, clients[idx].name);
                        if (new_fd > max_fd) {
                            max_fd = new_fd;
                        }
                    }
                }
            }
        }

        // 5) Sprawdzamy, czy któryś z aktywnych klientów wysłał wiadomość
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (!clients[i].active) continue;
            int sd = clients[i].socket_desc;
            if (FD_ISSET(sd, &read_fds)) {
                struct message inc;
                ssize_t r = read(sd, &inc, sizeof(inc));
                if (r <= 0) {
                    // Klient nagle rozłączył się
                    remove_client(i);
                } else {
                    switch (inc.type) {
                        case LIST: {
                            // Budujemy tekst z listą wszystkich aktywnych klientów
                            char buf[2048] = {0};
                            for (int j = 0; j < MAX_CLIENTS; j++) {
                                if (clients[j].active) {
                                    char line[64];
                                    format_client_info(&clients[j], line);
                                    strncat(buf, line, sizeof(buf) - strlen(buf) - 1);
                                }
                            }
                            struct message out;
                            out.type = MES;
                            out.sender_id = -1;   // od serwera
                            out.receiver_id = clients[i].id;
                            strncpy(out.content, buf, 100 - 1);
                            out.content[100 - 1] = '\0';
                            write(sd, &out, sizeof(out));
                            break;
                        }
                        case TO_ALL: {
                            // Rozsyłamy wiadomość do wszystkich poza nadawcą
                            time_t t = time(NULL);
                            char timestr[32];
                            strftime(timestr, sizeof(timestr),
                                     "%Y-%m-%d %H:%M:%S", localtime(&t));
                            /* content ma długość 100, więc buf ma 100+64 znaków */
                            char buf[100 + 64];
                            snprintf(buf, sizeof(buf),
                                     "[%s] %s: %s",
                                     timestr, clients[i].name, inc.content);
                            for (int j = 0; j < MAX_CLIENTS; j++) {
                                if (clients[j].active && j != i) {
                                    send_message_to_client(j, buf, clients[i].id);
                                }
                            }
                            break;
                        }
                        case TO_ONE: {
                            // inc.receiver_id = docelowy ID
                            int target_id = inc.receiver_id;
                            int idx_target = -1;
                            for (int j = 0; j < MAX_CLIENTS; j++) {
                                if (clients[j].active && clients[j].id == target_id) {
                                    idx_target = j;
                                    break;
                                }
                            }
                            if (idx_target != -1) {
                                time_t t = time(NULL);
                                char timestr[32];
                                strftime(timestr, sizeof(timestr),
                                         "%Y-%m-%d %H:%M:%S", localtime(&t));
                                char buf[100 + 64];
                                snprintf(buf, sizeof(buf),
                                         "[%s] %s (priv): %s",
                                         timestr, clients[i].name, inc.content);
                                send_message_to_client(idx_target, buf, clients[i].id);
                            }
                            break;
                        }
                        case STOP: {
                            // Klient deklaruje zakończenie
                            remove_client(i);
                            break;
                        }
                        default:
                            printf("[SERVER] Nieznany typ %d od klienta %d\n",
                                   inc.type, clients[i].id);
                    }
                }
            }
        }
    } // koniec while

    // (nigdy nie dojdziemy tu, bo cały czas pracujemy w pętli)
    close(listen_fd);
    return 0;
}
