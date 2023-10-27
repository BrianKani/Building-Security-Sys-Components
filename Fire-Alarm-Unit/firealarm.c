#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#define UDP_PORT 12345 
#define OVERSEER_PORT 8080  
#define OVERSEER_IP "127.0.0.1"  

// Define shared memory structure
struct SharedMemory {
    char alarm; // '-' if inactive, 'A' if active
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

// Define structure for fail-safe doors
struct Door {
    struct in_addr address;
    in_port_t port;
};

// Global variables for shared memory and door list
struct SharedMemory *sharedMem;
struct Door doors[100]; // Adjust the size as needed
int udpSocket;  // Declare the UDP socket

// Function to send "DREG" confirmation to the overseer
void sendDoorRegistrationConfirmation(int overseerSocket, struct in_addr doorAddress, in_port_t doorPort) {
    // Implement the logic to send the confirmation
}

// Function to handle fire emergency
void handleFireEmergency() {
    // Lock the mutex before accessing shared data
    pthread_mutex_lock(&sharedMem->mutex);

    // Implement the logic to trigger the alarm, open doors, and send notifications

    // Unlock the mutex when done
    pthread_mutex_unlock(&sharedMem->mutex);

    // Signal and wait using condition variables
    pthread_cond_signal(&sharedMem->cond);
    pthread_cond_wait(&sharedMem->cond, &sharedMem->mutex);
}

// Function to handle temperature updates
void handleTemperatureUpdate() {
    // Lock the mutex before accessing shared data
    pthread_mutex_lock(&sharedMem->mutex);

    // Implement the logic to check temperature and timestamp

    // Unlock the mutex when done
    pthread_mutex_unlock(&sharedMem->mutex);

    // Signal and wait using condition variables
    pthread_cond_signal(&sharedMem->cond);
    pthread_cond_wait(&sharedMem->cond, &sharedMem->mutex);
}

// Function to handle door registration
void handleDoorRegistration(int overseerSocket, struct sockaddr_in overseerAddr) {
    // Lock the mutex before accessing shared data
    pthread_mutex_lock(&sharedMem->mutex);

    // Implement the logic to add doors to the list and send confirmation

    // Unlock the mutex when done
    pthread_mutex_unlock(&sharedMem->mutex);

    // Signal and wait using condition variables
    pthread_cond_signal(&sharedMem->cond);
    pthread_cond_wait(&sharedMem->cond, &sharedMem->mutex);
}

int main(int argc, char *argv[]) {
    // Create a UDP socket to listen for incoming datagrams
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(UDP_PORT);  // Set the UDP port
    serverAddr.sin_addr.s_addr = INADDR_ANY;  // Listen on all available network interfaces

    // Bind the UDP socket to the specified port
    if (bind(udpSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind");
        close(udpSocket);
        exit(EXIT_FAILURE);
    }

    // Create a TCP socket for the overseer connection
    int overseerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (overseerSocket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in overseerAddr;
    memset(&overseerAddr, 0, sizeof(overseerAddr));
    overseerAddr.sin_family = AF_INET;
    overseerAddr.sin_port = htons(OVERSEER_PORT);  // Define the port
    overseerAddr.sin_addr.s_addr = inet_addr(OVERSEER_IP);  // Define the IP address

    // Connect to the overseer
    if (connect(overseerSocket, (struct sockaddr*)&overseerAddr, sizeof(overseerAddr)) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    // Send an initialization message (customize this)
    char initMessage[] = "INIT";
    send(overseerSocket, initMessage, strlen(initMessage), 0);

    // Main loop to listen for UDP datagrams
    while (1) {
        // Define a buffer to receive the datagram
        char buffer[1024];
        struct sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);

        // Receive the UDP datagram
        ssize_t recvResult = recvfrom(udpSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &addrLen);

        if (recvResult < 0) {
            perror("recvfrom");
            // Handle the error
        } else {
            // Datagram received successfully
        // Datagram received successfully
	if (strcmp(buffer, "FIRE") == 0) {
    		handleFireEmergency();
	} else if (strcmp(buffer, "TEMP") == 0) {
    	handleTemperatureUpdate();
	} else if (strcmp(buffer, "DOOR") == 0) {
    	handleDoorRegistration(overseerSocket, overseerAddr);
	} else {
    	// Handle other or unknown datagram types
	}
        }
    }

    // Clean up resources, close sockets, and release shared memory

    return 0;
}

