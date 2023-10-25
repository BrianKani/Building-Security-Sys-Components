#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>

typedef struct {
    char scanned[16];
    pthread_mutex_t mutex;
    pthread_cond_t scanned_cond;
    
    char response; // 'Y' or 'N' (or '\0' at first)
    pthread_cond_t response_cond;
} shm_cardreader;

int main(int argc, char **argv)
{
    if (argc < 6) {
        fprintf(stderr, "usage: {id} {wait time (in microseconds)} {shared memory path} {shared memory offset} {overseer address:port}\n");
        exit(1);
    }

    int id = atoi(argv[1]);
    int waittime = atoi(argv[2]);
    const char *shm_path = argv[3];
    int shm_offset = atoi(argv[4]);
    const char *overseer_addr = argv[5];
    
    int shm_fd = shm_open(shm_path, O_RDWR, 0);
    if (shm_fd == -1) {
        perror("shm_open()");
        exit(1);
    }

    struct stat shm_stat;
    if (fstat(shm_fd, &shm_stat) == -1) {
        perror("fstat()");
        exit(1);
    }
    
    printf("Shared memory file size: %ld\n", shm_stat.st_size);

    char *shm = mmap(NULL, shm_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap()");
        exit(1);
    }
    shm_cardreader *shared = (shm_cardreader *)(shm + shm_offset);
    int overseer_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (overseer_socket == -1) {
        perror("socket()");
        exit(1);
    }
    struct sockaddr_in overseer_server;
    overseer_server.sin_family = AF_INET;
    overseer_server.sin_port = htons(overseer_port);
    if (inet_pton(AF_INET, overseer_addr, &overseer_server.sin_addr) <= 0) {
        perror("inet_pton()");
        exit(1);
    }
    if (connect(overseer_socket, (struct sockaddr *)&overseer_server, sizeof(overseer_server)) == -1) {
        perror("connect()");
        exit(1);
    }

    pthread_mutex_lock(&shared->mutex);
    for (;;) {
        if (shared->scanned[0] != '\0') {
            char buf[17];
            memcpy(buf, shared->scanned, 16);
            buf[16] = '\0';
            printf("Scanned %s\n", buf);
            send(overseer_socket, buf, 16, 0);
            shared->response = 'Y';
            pthread_cond_signal(&shared->response_cond);
        }
        pthread_cond_wait(&shared->scanned_cond, &shared->mutex);
    }

    close(overseer_socket);
    close(shm_fd);

    

    return 0;
}
