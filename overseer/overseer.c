#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "shared_structs.h" // To be created later
#include <netinet/in.h>
#include <sys/socket.h>

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
int overseerPort;

// Function to handle incoming TCP connections
void* handleTCPConnections(void* arg) {
    int serverSocket, newSocket;
    struct sockaddr_in serverAddress, newAddress;
    socklen_t addrSize;
    char buffer[1024];

    // Create a socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error creating server socket");
        exit(1);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(overseerPort);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Error binding server socket");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 10) == 0) {
        printf("Listening for incoming connections...\n");
    } else {
        perror("Error listening for connections");
        exit(1);
    }

    addrSize = sizeof(newAddress);
    while (true) {
        newSocket = accept(serverSocket, (struct sockaddr*)&newAddress, &addrSize);

        if (newSocket < 0) {
            perror("Error accepting connection");
            exit(1);
        }

        recv(newSocket, buffer, 1024, 0);
        printf("Received from client: %s\n");

        send(newSocket, "Message received", strlen("Message received"), 0);

        close(newSocket);
    }
}

// Function to handle incoming UDP datagrams
void* handleUDPDatagrams(void* arg) {
    int udpSocket;
    struct sockaddr_in serverAddress;
    socklen_t addrSize;
    char buffer[1024];

    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1) {
        perror("Error creating UDP socket");
        exit(1);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(overseerPort);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(udpSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Error binding UDP socket");
        exit(1);
    }

    addrSize = sizeof(serverAddress);
    while (true) {
        recvfrom(udpSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&serverAddress, &addrSize);
        printf("Received UDP datagram: %s\n");
    }
}

// Function to initialize the overseer component
void initializeOverseer(char* addressPort, int doorOpenDuration, int datagramResendDelay, char* authFile, char* connFile, char* layoutFile, char* sharedMemPath, int sharedMemOffset) {
    sharedMem = (struct SharedMemory*)malloc(sizeof(struct SharedMemory));
    sharedMem->security_alarm = '-';
    pthread_mutex_init(&(sharedMem->mutex), NULL);
    pthread_cond_init(&(sharedMem->cond), NULL);

    doorOpenDuration = doorOpenDuration;
    datagramResendDelay = datagramResendDelay;
    authFile = authFile;
    connFile = connFile;
    layoutFile = layoutFile; 
}

int main(int argc, char* argv[]) {
    overseerPort = atoi(argv[1]);

    pthread_t tcpThread, udpThread;
    pthread_create(&tcpThread, NULL, handleTCPConnections, NULL);
    pthread_create(&udpThread, NULL, handleUDPDatagrams, NULL);

    void initializeOverseer(char* addressPort, int doorDuration, int datagramDelay, char* authFileName, char* connFileName, char* layoutFileName, char* sharedMemPath, int sharedMemOffset) {
    }
}

