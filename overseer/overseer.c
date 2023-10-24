#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

// Define the shared memory structure
struct SharedMemory {
    char security_alarm; // '-' if inactive, 'A' if active
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

// Global variables
struct SharedMemory* sharedMem;
int doorOpenDuration;
int datagramResendDelay;
char* authFile;
char* connFile;
char* layoutFile;

typedef struct {
    int socket;
    struct sockaddr_in address;
} TCPClient;

void* handleTCPConnections(void* arg) {
    TCPClient* client = (TCPClient*)arg;
    int clientSocket = client->socket;

    char buffer[1024];

    while (1) {
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesRead <= 0) {
            shutdown(clientSocket, SHUT_RDWR); // Shutdown both sending and receiving
            close(clientSocket);
            free(client);
            pthread_exit(NULL);
        }

        if (strncmp(buffer, "SCANNED", 7) == 0) {
            // Process the received data as needed
            const char* response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
            send(clientSocket, response, strlen(response), 0);
        }

        memset(buffer, 0, sizeof(buffer));
    }

    return NULL;
}

// Structure to represent UDP data
typedef struct {
    int socket;
    struct sockaddr_in address;
} UDPData;

void* handleUDPDatagrams(void* arg) {
    UDPData* udpData = (UDPData*)arg;

    int udpSocket = udpData->socket;

    char buffer[1024]; // Adjust buffer size as needed

    while (1) {
        ssize_t bytesRead;
        socklen_t addressSize = sizeof(udpData->address);

        bytesRead = recvfrom(udpSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&(udpData->address), &addressSize);

        if (bytesRead <= 0) {
            continue; // Or consider terminating the thread
        }

        // Process the received data as needed
        printf("Received UDP message from %s:%d: %s\n",
               inet_ntoa(udpData->address.sin_addr),
               ntohs(udpData->address.sin_port),
               buffer);

        memset(buffer, 0, sizeof(buffer));
    }

    return NULL;
}

void initializeOverseer(char* addressPort, int doorOpenDuration, int datagramResendDelay, char* authFile, char* connFile, char* layoutFile, char* sharedMemPath, int sharedMemOffset) {
    // Initialize shared memory
    key_t shm_key = ftok(sharedMemPath, sharedMemOffset);
    int shm_id = shmget(shm_key, sizeof(struct SharedMemory), 0666 | IPC_CREAT);

    if (shm_id == -1) {
        perror("Error creating shared memory");
        exit(1);
    }

    sharedMem = (struct SharedMemory*)shmat(shm_id, NULL, 0);

    // Set global configuration variables
    doorOpenDuration = doorOpenDuration;
    datagramResendDelay = datagramResendDelay;
    authFile = authFile;
    connFile = connFile;
    layoutFile = layoutFile;
}

int main(int argc, char* argv[]) {
    if (argc < 9) {
        fprintf(stderr, "Usage: overseer {address:port} {door open duration} {datagram resend delay} {auth file} {conn file} {layout file} {shared memory path} {shared memory offset}\n");
        exit(1);
    }

    // Parse command-line arguments
    char* addressPort = argv[1];
    doorOpenDuration = atoi(argv[2]);
    datagramResendDelay = atoi(argv[3]);
    authFile = argv[4];
    connFile = argv[5];
    layoutFile = argv[6];

    // Initialize shared memory
    key_t shm_key = ftok(argv[7], atoi(argv[8]));
    int shm_id = shmget(shm_key, sizeof(struct SharedMemory), 0666 | IPC_CREAT);
    sharedMem = (struct SharedMemory*)shmat(shm_id, NULL, 0);

    // Create threads for handling TCP connections and UDP datagrams
    pthread_t tcpThread, udpThread;
    pthread_create(&tcpThread, NULL, handleTCPConnections, NULL);
    pthread_create(&udpThread, NULL, handleUDPDatagrams, NULL);

  

    // Cleanup and exit
    pthread_mutex_destroy(&(sharedMem->mutex));
    pthread_cond_destroy(&(sharedMem->cond));
    shmdt(sharedMem);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}
