#ifndef SHARED_STRUCTS_H
#define SHARED_STRUCTS_H

#include <pthread.h>

// Define shared memory structure for the Door Controller
struct DoorControllerSharedMemory {
    char security_alarm; // '-' if inactive, 'A' if active
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

// Define shared memory structure for the Fire Alarm Unit
struct FireAlarmUnitSharedMemory {
    char alarm; // '-' if inactive, 'A' if active
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

// Define shared memory structure for the Card Reader Controller
struct CardReaderSharedMemory {
    char scanned[16];
    pthread_mutex_t mutex;
    pthread_cond_t scanned_cond;

    char response; // 'Y' or 'N' (or '\0' at first)
    pthread_cond_t response_cond;
};

// Add more shared memory structures as needed for other components

#endif // SHARED_STRUCTS_H
