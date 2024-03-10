#include <stdio.h>
#include <error.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <csignal>

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>

#include <thread>
#include <chrono>

#define PERMISSION_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

void lock(int semid)
{
    struct sembuf operation = {0, -1, 0};
    if (semop(semid, &operation, 1) == -1)
    {
        perror("semop");
        exit(1);
    }
}

void unlock(int semid)
{
    struct sembuf operation = {0, 1, 0};
    if (semop(semid, &operation, 1) == -1)
    {
        perror("semop");
        exit(1);
    }
}

struct ThreadParams
{
    const int semId;
    const int segReadId;
};

void *read(void *threadParams)
{
    ThreadParams *params = (ThreadParams *)(threadParams);
    char *ptr;

    while (true)
    {
        lock(params->semId);

        ptr = (char *)shmat(params->segReadId, NULL, 0);

        // Сheck for yours
        char pidString[7];
        int arrayId = 0;
        while (*ptr != ':')
        {
            pidString[arrayId++] = *ptr;
            ptr++;
        }
        ptr++;

        pid_t pid = getpid();

        if (pid != std::stoi(pidString))
        {
            std::cout << "From " << pidString << ": " << ptr << std::endl;
        }

        unlock(params->semId);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void *write(void *threadParams)
{
    ThreadParams *params = (ThreadParams *)(threadParams);

    char *ptr;
    pid_t pid = getpid();

    while (true)
    {

        char inputString[1000];
        std::cin >> inputString;

        char newString[1007];
        sprintf(newString, "%d:%s", pid, inputString);

        ptr = (char *)shmat(params->segReadId, NULL, 0);

        std::strcpy(ptr, newString);

        unlock(params->semId);
        lock(params->semId);
    }
}

int main(int argc, char **argv)
{
    // Create file for semaphore
    char sharedFileName[] = "/tmp/com.sharedMemory.file";
    std::ofstream file(sharedFileName);
    if (file.is_open())
    {
        file.close();
    }
    else
    {
        std::cerr << "Error creation semaphore file. Try reinstalling the application." << std::endl;
        return 1;
    }

    int oflag = PERMISSION_MODE | IPC_CREAT; // or 0666 | IPC_CREAT

    key_t semKey = ftok(sharedFileName, 'S');
    int semId = semget(semKey, 1, 0666 | IPC_CREAT);

    if (semId == -1)
    {
        std::cerr << "Semaphore creation error" << std::endl;
        return 1;
    }

    key_t segKey = ftok(sharedFileName, 'R');
    int segId = shmget(segKey, 1007, oflag);

    if (segId == -1)
    {
        std::cout << "Error accessing shared memory segment" << std::endl;
        return 1;
    }

    std::cout << "You are ready to receive and send messages" << std::endl;
    std::cout << "Messages must be no more than 1000 characters long. For spaces use '_'." << std::endl;

    ThreadParams *threadParam = new ThreadParams{semId, segId};
    pthread_t writeThread, readThread;

    if (pthread_create(&readThread, NULL, read, (void *)threadParam) != 0)
    {
        std::cerr << "Error creating thread 1" << std::endl;
        return 1;
    }

    if (pthread_create(&writeThread, NULL, write, (void *)threadParam) != 0)
    {
        std::cerr << "Error creating thread 2" << std::endl;
        return 1;
    }

    pthread_join(readThread, NULL);
    pthread_join(writeThread, NULL);

    shmctl(segId, IPC_RMID, NULL);
    semctl(semId, 0, IPC_RMID);

    return 0;
}