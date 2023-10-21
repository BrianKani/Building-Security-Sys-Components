#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

// Define shared memory structure
struct SharedMemory {
    char status; // 'O' for open, 'C' for closed, 'o' for opening, 'c' for closing
    pthread_mutex_t mutex;
    pthread_cond_t cond_start;
    pthread_cond_t cond_end;
};

// Global variable for shared memory
struct SharedMemory *sharedMem;

// Function to send initialization message to the overseer
void sendInitializationMessage(int overseerSocket, int id, const char* doorConfig) {
    char message[50];
    snprintf(message, sizeof(message), "DOOR %d %s %s#", id, doorConfig, doorConfig);

    // Implement the logic to send the initialization message
}

// Function to open the door
void openDoor(int clientSocket, int id) {
    // Lock the mutex
    pthread_mutex_lock(&sharedMem->mutex);

    // Check the door status
    if (sharedMem->status == 'C') {
        // Door is closed
        // Respond with OPENING#
        send(clientSocket, "OPENING#", 8, 0);

        // Set the door status to 'o' (opening)
        sharedMem->status = 'o';

        // Signal cond_start
        pthread_cond_signal(&sharedMem->cond_start);

        // Wait for the door to open
        pthread_cond_wait(&sharedMem->cond_end, &sharedMem->mutex);

        // Respond with OPENED# when the door is open
        send(clientSocket, "OPENED#", 7, 0);
    } else if (sharedMem->status == 'O') {
        // Door is already open
        send(clientSocket, "ALREADY#", 8, 0);
    } else {
        // Door is in the process of opening or closing
        send(clientSocket, "BUSY#", 5, 0);
    }

    // Unlock the mutex
    pthread_mutex_unlock(&sharedMem->mutex);

    // Close the connection
    close(clientSocket);
}

// Function to close the door
void closeDoor(int clientSocket, int id) {
    // Lock the mutex
    pthread_mutex_lock(&sharedMem->mutex);

    // Check the door status
    if (sharedMem->status == 'O') {
        // Door is open
        // Respond with CLOSING#
        send(clientSocket, "CLOSING#", 8, 0);

        // Set the door status to 'c' (closing)
        sharedMem->status = 'c';

        // Signal cond_start
        pthread_cond_signal(&sharedMem->cond_start);

        // Wait for the door to close
        pthread_cond_wait(&sharedMem->cond_end, &sharedMem->mutex);

        // Respond with CLOSED# when the door is closed
        send(clientSocket, "CLOSED#", 7, 0);
    } else if (sharedMem->status == 'C') {
        // Door is already closed
        send(clientSocket, "ALREADY#", 8, 0);
    } else {
        // Door is in the process of opening or closing
        send(clientSocket, "BUSY#", 5, 0);
    }

    // Unlock the mutex
    pthread_mutex_unlock(&sharedMem->mutex);

    // Close the connection
    close(clientSocket);
}

int main(int argc, char *argv[]) {
    // Parse and validate command-line arguments
    int id;
    char* doorConfig;
    char* sharedMemoryPath;
    int sharedMemoryOffset;
    char* overseerAddress;
    int overseerPort;

    if (argc != 6) {
        fprintf(stderr, "Usage: %s {id} {address:port} {FAIL_SAFE | FAIL_SECURE} {shared memory path} {shared memory offset} {overseer address:port}\n", argv[0]);
        exit(1);
    }

    id = atoi(argv[1);
    doorConfig = argv[3];
    sharedMemoryPath = argv[4];
    sharedMemoryOffset = atoi(argv[5]);
    overseerAddress = strtok(argv[6], ":");
    overseerPort = atoi(strtok(NULL, ":"));

    // Open and map shared memory
    int shm_fd = shm_open(sharedMemoryPath, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    sharedMem = (struct SharedMemory*)mmap(0, sizeof(struct SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, sharedMemoryOffset);
    if (sharedMem == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Create a TCP socket for the door controller
    int doorSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (doorSocket == -1) {
        perror("socket");
        exit(1);
    }

    // Set up the sockaddr_in structure
    struct sockaddr_in doorAddr;
    memset(&doorAddr, 0, sizeof(doorAddr));
    doorAddr.sin_family = AF_INET;
    doorAddr.sin_port = htons(overseerPort);
    inet_pton(AF_INET, overseerAddress, &(doorAddr.sin_addr));

    // Bind the socket
    if (bind(doorSocket, (struct sockaddr*)&doorAddr, sizeof(doorAddr)) == -1) {
        perror("bind");
        exit(1);
    }

    // Start listening for incoming connections
    if (listen(doorSocket, 10) == -1) {
        perror("listen");
        exit(1);
    }

    // Send initialization message to the overseer
    int overseerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (overseerSocket >= 0) {
        struct sockaddr_in overseerAddr;
        memset(&overseerAddr, 0, sizeof(overseerAddr));
        overseerAddr.sin_family = AF_INET;
        overseerAddr.sin_port = htons(overseerPort);
        inet_pton(AF_INET, overseerAddress, &(overseerAddr.sin_addr));

        if (connect(overseerSocket, (struct sockaddr*)&overseerAddr, sizeof(overseerAddr)) == 0) {
            sendInitializationMessage(overseerSocket, id, doorConfig);
            close(overseerSocket);
        }
    }

    // Main loop for normal operation
    while (1) {
        // Accept incoming connections
        struct sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);
        int clientSocket = accept(doorSocket, (struct sockaddr*)&clientAddr, &addrLen);
        if (clientSocket == -1) {
            perror("accept");
            continue;
        }

        char requestBuffer[64];
        int bytesReceived = recv(clientSocket, requestBuffer, sizeof(requestBuffer), 0);
        if (bytesReceived > 0) {
            requestBuffer[bytesReceived] = '\0';
            if (strstr(requestBuffer, "OPEN#") != NULL) {
                openDoor(clientSocket, id);
            } else if (strstr(requestBuffer, "CLOSE#") != NULL) {
                closeDoor(clientSocket, id);
            } else {
                // Handle other messages or errors
            }
        } else {
            perror("recv");
        }
    }

    // Clean up resources, close sockets, and release shared memory
    munmap(sharedMem, sizeof(struct SharedMemory));
    close(shm_fd);
    close(doorSocket);

    return 0;
}

