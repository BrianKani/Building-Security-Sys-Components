#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h> // Include headers for shared memory operations

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
            shutdown(clientSocket, SHUT_RDWR);  // Shutdown both sending and receiving
            close(clientSocket);
            free(client);
            pthread_exit(NULL);
        }

        if (strncmp(buffer, "SCANNED", 7) == 0) {
            // Process the received data as needed
            // Example: Send a response
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

// Function to handle incoming UDP datagrams
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
        memset(buffer, 0, sizeof(buffer));
    }

    return NULL;
}

typedef struct {
    int socket;
    struct sockaddr_in address;
} UDPData;

// Function to handle incoming UDP datagrams
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

        memset(buffer, 0, sizeof(buffer);
    }

    return NULL;
}
// Function to initialize the overseer component
void initializeOverseer(char* addressPort, int doorOpenDuration, int datagramResendDelay, char* authFile, char* connFile, char* layoutFile, char* sharedMemPath, int sharedMemOffset) {
    // Initialize shared memory
    key_t shm_key = ftok(sharedMemPath, sharedMemOffset);
    int shm_id = shmget(shm_key, sizeof(struct SharedMemory), 0666 | IPC_CREAT); // Add IPC_CREAT

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
    
    // Implement other initialization logic here
}

// Function to process SCANNED messages from card readers
void processScannedMessage(char* message) {
    // Implement card access control logic here
}

// Function to raise a security alarm manually
void raiseSecurityAlarm() {
    // Implement security alarm logic here
}

// Main function
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

	
    int serverSocket; // Server socket file descriptor
    struct sockaddr_in serverAddr; // Address structure for the server (IPv4)
    int port = 8080;
    int backlog = 5; // Maximum number of pending connections

    // Create a socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("socket");
        exit(1);
    }

    // Initialize the server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Bind to any available network interface
    serverAddr.sin_port = htons(port);

    // Bind the socket to the specified address
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("bind");
        close(serverSocket);
        exit(1);
    }

    // Listen for incoming connections
    if (listen(serverSocket, backlog) == -1) {
        perror("listen");
        close(serverSocket);
        exit(1);
    }

    printf("Server is listening on port %d...\n", port);

    while (1) {
        // Accept incoming connections
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);

        if (clientSocket == -1) {
            perror("accept");
            continue;
        }

        // Create a new thread to handle the client connection
        TCPClient* client = (TCPClient*)malloc(sizeof(TCPClient));
        client->socket = clientSocket;
        client->address = clientAddr;

        pthread_t thread;
        if (pthread_create(&thread, NULL, handleTCPConnections, client) != 0) {
            perror("pthread_create");
            free(client);
            close(clientSocket);
            continue;
        }
    }

    close(serverSocket);
    return 0;
}

    int udpSocket; // UDP socket file descriptor
    struct sockaddr_in serverAddr; // Address structure for the server (IPv4)
    int port = 8080;


    // Create a UDP socket
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1) {
        perror("socket");
        exit(1);
    }

    // Initialize the server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Bind to any available network interface
    serverAddr.sin_port = htons(port);

    // Bind the UDP socket to the specified address
    if (bind(udpSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("bind");
        close(udpSocket);
        exit(1);
    }

    printf("UDP server is listening on port %d...\n", port);

    while (1) {
        UDPData* udpData = (UDPData*)malloc(sizeof(UDPData));
        udpData->socket = udpSocket;

        pthread_t thread;
        if (pthread_create(&thread, NULL, handleUDPDatagrams, udpData) != 0) {
            perror("pthread_create");
            free(udpData);
        }
    }

    // You can close the UDP socket when done, but in this example, the server runs indefinitely.
    // close(udpSocket);

    return 0;
}


    // Initialize the overseer component
initializeOverseer(addressPort, doorOpenDuration, datagramResendDelay, authFile, connFile, layoutFile, argv[7], atoi(argv[8]));

    void initializeOverseer(char* addressPort, int doorOpenDuration, int datagramResendDelay, char* authFile, char* connFile, char* layoutFile, char* sharedMemPath, int sharedMemOffset) {
    // Initialize shared memory
    key_t shm_key = ftok(sharedMemPath, sharedMemOffset);
    int shm_id = shmget(shm_key, sizeof(struct SharedMemory), 0666 | IPC_CREAT); // Add IPC_CREAT

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
    // Cleanup and exit
    pthread_mutex_destroy(&(sharedMem->mutex));
    pthread_cond_destroy(&(sharedMem->cond));
    shmdt(sharedMem);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}

