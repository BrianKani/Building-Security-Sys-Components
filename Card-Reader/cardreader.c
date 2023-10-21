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
    char scanned[16];
    pthread_mutex_t mutex;
    pthread_cond_t scanned_cond;

    char response; // 'Y' or 'N' (or '\0' at first)
    pthread_cond_t response_cond;
};

// Global variable for shared memory
struct SharedMemory *sharedMem;

// Function to send initialization message to the overseer
void sendInitializationMessage(int overseerSocket, int id) {
    char message[50];
    snprintf(message, sizeof(message), "CARDREADER %d HELLO#", id);
    
    // Implement the logic to send the initialization message
}

// Function to open a TCP connection to the overseer and send data
char sendScannedData(int overseerSocket, int id) {
    char response = 'N'; // Default to 'N' for not allowed
    
    // Lock the mutex and check the scanned code
    pthread_mutex_lock(&sharedMem->mutex);
    if (sharedMem->scanned[0] != '\0') {
        // Create a TCP connection to the overseer
        int overseerSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (overseerSocket >= 0) {
            struct sockaddr_in overseerAddr;
            memset(&overseerAddr, 0, sizeof(overseerAddr));
            overseerAddr.sin_family = AF_INET;
            overseerAddr.sin_port = htons(overseerPort);
            inet_pton(AF_INET, overseerAddress, &(overseerAddr.sin_addr));

            if (connect(overseerSocket, (struct sockaddr*)&overseerAddr, sizeof(overseerAddr)) == 0) {
                // Prepare and send the message
                char message[50];
                snprintf(message, sizeof(message), "CARDREADER %d SCANNED %s#", id, sharedMem->scanned);

                if (send(overseerSocket, message, strlen(message), 0) != -1) {
                    // Successfully sent the message
                    response = 'Y'; // Set response to 'Y' for allowed
                }

                // Close the connection
                close(overseerSocket);
            }
        }
    }

    // Update the response and signal response_cond
    sharedMem->response = response;
    pthread_cond_signal(&sharedMem->response_cond);

    // Unlock the mutex
    pthread_mutex_unlock(&sharedMem->mutex);

    return response;
}

int main(int argc, char *argv[]) {
    // Parse and validate command-line arguments
    int id;
    char* sharedMemoryPath;
    int sharedMemoryOffset;
    char* overseerAddress;
    int overseerPort;

    if (argc != 6) {
        fprintf(stderr, "Usage: %s {id} {shared memory path} {shared memory offset} {overseer address} {overseer port}\n", argv[0]);
        exit(1);
    }

    id = atoi(argv[1]);
    sharedMemoryPath = argv[2];
    sharedMemoryOffset = atoi(argv[3]);
    overseerAddress = argv[4];
    overseerPort = atoi(argv[5]);

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

    // Send initialization message to the overseer
    int overseerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (overseerSocket >= 0) {
        struct sockaddr_in overseerAddr;
        memset(&overseerAddr, 0, sizeof(overseerAddr));
        overseerAddr.sin_family = AF_INET;
        overseerAddr.sin_port = htons(overseerPort);
        inet_pton(AF_INET, overseerAddress, &(overseerAddr.sin_addr));

        if (connect(overseerSocket, (struct sockaddr*)&overseerAddr, sizeof(overseerAddr)) == 0) {
            sendInitializationMessage(overseerSocket, id);
            close(overseerSocket);
        }
    }

    // Main loop for normal operation
    while (1) {
        sendScannedData(overseerSocket, id);
    }

    // Clean up resources, close sockets, and release shared memory
    munmap(sharedMem, sizeof(struct SharedMemory));
    close(shm_fd);

    return 0;
}
