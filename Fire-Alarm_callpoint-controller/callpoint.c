/* This C program implements the Fire Alarm Call-Point Controller as a safety-critical component, following the provided specifications and including appropriate safety-critical software standards. It ensures that fire emergency datagrams are sent to the Fire Alarm Unit in a loop, while also waiting for any changes in the shared memory status.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// Define shared memory structure
struct SharedMemory {
    char status; // '-' for inactive, '*' for active
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

// Global variable for shared memory
struct SharedMemory *sharedMem;

// Function to send a fire emergency datagram
void sendFireEmergency(int fireAlarmSocket, const char* fireAlarmAddress, int fireAlarmPort) {
    // Construct the fire emergency datagram
    char fireDatagram[4] = {'F', 'I', 'R', 'E'};

    // Set up the sockaddr_in structure for the fire alarm unit
    struct sockaddr_in fireAlarmAddr;
    memset(&fireAlarmAddr, 0, sizeof(fireAlarmAddr));
    fireAlarmAddr.sin_family = AF_INET;
    fireAlarmAddr.sin_port = htons(fireAlarmPort);
    fireAlarmAddr.sin_addr.s_addr = inet_addr(fireAlarmAddress);

    // Send the datagram to the fire alarm unit
    sendto(fireAlarmSocket, fireDatagram, sizeof(fireDatagram), 0, (struct sockaddr*)&fireAlarmAddr, sizeof(fireAlarmAddr));
}

int main(int argc, char *argv[]) {
    // Parse and validate command-line arguments
    if (argc != 5) {
        fprintf(stderr, "Usage: %s {resend delay (in microseconds)} {shared memory path} {shared memory offset} {fire alarm unit address:port}\n", argv[0]);
        exit(1);
    }

    int resendDelay = atoi(argv[1]);
    char* sharedMemoryPath = argv[2];
    int sharedMemoryOffset = atoi(argv[3]);
    char* fireAlarmAddress = strtok(argv[4], ":");
    int fireAlarmPort = atoi(strtok(NULL, ":"));

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

    // Create a UDP socket for the fire alarm call-point controller
    int fireAlarmSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (fireAlarmSocket == -1) {
        perror("socket");
        exit(1);
    }

    // Main loop for normal operation
    while (1) {
        // Lock the mutex
        pthread_mutex_lock(&sharedMem->mutex);

        // Check 'status'
        if (sharedMem->status == '*') {
            // Send a fire emergency datagram
            sendFireEmergency(fireAlarmSocket, fireAlarmAddress, fireAlarmPort);

            // Sleep for {resend delay} microseconds
            usleep(resendDelay);
        } else {
            // Wait on 'cond'
            pthread_cond_wait(&sharedMem->cond, &sharedMem->mutex);
        }

        // Unlock the mutex
        pthread_mutex_unlock(&sharedMem->mutex);
    }

    // Clean up resources and close the socket
    munmap(sharedMem, sizeof(struct SharedMemory));
    close(shm_fd);
    close(fireAlarmSocket);

    return 0;
}

