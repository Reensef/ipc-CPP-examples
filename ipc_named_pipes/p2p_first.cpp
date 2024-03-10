#include <stdio.h>
#include <error.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>

#include <thread>
#include <chrono>

#define PERMISSION_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

struct ThreadParams
{
    char *namePipeOne;
    char *namePipeTwo;
};

void *read(void *threadParams)
{
    ThreadParams *params = (ThreadParams *)(threadParams);

    while (true)
    {
        char data[1000];

        int id = open(params->namePipeTwo, O_RDONLY);

        if (id == -1)
        {
            std::cout << "Error open pipe" << std::endl;
        }

        read(id, data, 1000);
        close(id);

        std::cout << "Recived message: " << data << std::endl;
    }
}

void *write(void *threadParams)
{
    ThreadParams *params = (ThreadParams *)(threadParams);

    while (true)
    {

        char inputString[1000];
        std::cin >> inputString;

        int id = open(params->namePipeOne, O_WRONLY);

        if (id == -1)
        {
            std::cout << "Error open pipe" << std::endl;
        }

        write(id, inputString, 1000);

        close(id);
    }
}

int main(int argc, char **argv)
{
    char namePipeOne[] = "/tmp/fifo.1";
    char namePipeTwo[] = "/tmp/fifo.2";

    mkfifo(namePipeOne, PERMISSION_MODE);
    mkfifo(namePipeTwo, PERMISSION_MODE);

    std::cout << "You are ready to receive and send messages:" << std::endl;
    std::cout << "Messages must be no more than 1000 characters long. For spaces use '_'." << std::endl;

    ThreadParams *threadParam = new ThreadParams{namePipeOne, namePipeTwo};
    pthread_t writeThread, readThread;

    if (pthread_create(&readThread, NULL, read, (void *)threadParam) != 0)
    {
        std::cerr << "Error creating readThread. Try reinstalling the application." << std::endl;
        return 1;
    }

    if (pthread_create(&writeThread, NULL, write, (void *)threadParam) != 0)
    {
        std::cerr << "Error creating thread writeThread. Try reinstalling the application." << std::endl;
        return 1;
    }

    pthread_join(readThread, NULL);
    pthread_join(writeThread, NULL);

    unlink(namePipeOne);
    unlink(namePipeTwo);

    return 0;
}