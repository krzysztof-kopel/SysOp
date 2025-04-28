#include <sys/msg.h>

int main() {
    key_t key = ftok("key_file.txt", 'A');
    int queue = msgget(key, IPC_CREAT | 0666);

    
}