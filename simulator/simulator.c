#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <pthread.h>

// Define shared memory structure for the overseer
struct OverseerSharedMemory {
    pthread_mutex_t mutex;
    int security_alarm;
};

int main() {
    key_t overseer_key = ftok("keyfile", 1); // Generate a key for shared memory
    int overseer_shmid = shmget(overseer_key, sizeof(struct OverseerSharedMemory), IPC_CREAT | 0666);
    if (overseer_shmid == -1) {
        perror("shmget");
        exit(1);
    }

    struct OverseerSharedMemory *overseer_memory = shmat(overseer_shmid, NULL, 0);
    if (overseer_memory == (void*)-1) {
        perror("shmat");
        exit(1);
    }

    // Initialize mutex
    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&(overseer_memory->mutex), &mutex_attr);

    // Initialize other variables
    overseer_memory->security_alarm = 0;

    int child_pid = fork();

    if (child_pid == 0) {
        // Child process (overseer)
        while (1) {
            // Simulate overseer logic here
            // For example, read events, update shared memory, and handle user input

            // Lock the mutex and perform overseer operations
            pthread_mutex_lock(&(overseer_memory->mutex));
            overseer_memory->security_alarm = 1;
            // Unlock the mutex
            pthread_mutex_unlock(&(overseer_memory->mutex));

            // Simulate some time before the next iteration
            usleep(1000000); // Sleep for 1 second
        }
    } else if (child_pid > 0) {
        // Parent process (simulator)
        // Simulate the scenario, manage other components, and handle timing

        // Wait for a while (1 second) before terminating the overseer
        usleep(1000000);

        // Terminate the overseer
        kill(child_pid, SIGTERM);

        // Clean up shared memory
        shmdt(overseer_memory);
        shmctl(overseer_shmid, IPC_RMID, NULL);
    } else {
        perror("fork");
    }

    return 0;
}
