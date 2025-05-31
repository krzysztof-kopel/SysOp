#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define NEW 1
#define LIST 2
#define TO_ALL 3
#define TO_ONE 4
#define STOP 5
#define ALIVE 6
#define MES 7
#define PING 8

struct client {
    int id;
    char name[50];
    int socket_desc;
    int active;
};

struct message {
    int type;
    int sender_id;
    int receiver_id;
    char content[100];
};

struct client_list_message {
    struct client clients[100];
};
