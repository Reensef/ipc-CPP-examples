#include <stdio.h>
#include <error.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
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
    const int msgNameFirst;
    const int msgNameSecond;
};

struct Message
{
    long mtype;
    char mtext[1000];
};

void *read(void *threadParams)
{
    ThreadParams *params = (ThreadParams *)(threadParams);

    while (true)
    {
        pid_t pid = getpid();
        Message msg;

        msgrcv(params->msgNameFirst, &msg, sizeof(msg.mtext), 1, 0);

        std::cout << "From " << pid << ": " << msg.mtext << " with type " << msg.mtype << std::endl;
    }
}

void *write(void *threadParams)
{
    ThreadParams *params = (ThreadParams *)(threadParams);

    while (true)
    {

        char inputString[1000];
        std::cin >> inputString;

        Message msg;
        msg.mtype = 1;
        strcpy(msg.mtext, inputString);

        msgsnd(params->msgNameSecond, &msg, sizeof(msg.mtext), 0);
    }
}

int main(int argc, char **argv)
{
    char msgNameFirst[] = "/tmp/messageQueue.1";
    char msgNameSecond[] = "/tmp/messageQueue.2";

    std::ofstream file(msgNameFirst);
    if (file.is_open())
    {
        file.close();
    }
    else
    {
        std::cerr << "Error creation message queue file. Try reinstalling the application." << std::endl;
        return 1;
    }
    std::ofstream file2(msgNameSecond);
    if (file2.is_open())
    {
        file2.close();
    }
    else
    {
        std::cerr << "Error creation message queue file. Try reinstalling the application." << std::endl;
        return 1;
    }

    key_t msgKeyFirst = ftok(msgNameFirst, 'R');
    int msgIdFirst = msgget(msgKeyFirst, 0666 | IPC_CREAT);

    key_t msgKeySecond = ftok(msgNameSecond, 'R');
    int msgIdSecond = msgget(msgKeySecond, 0666 | IPC_CREAT);

    std::cout << "You are ready to receive and send messages:" << std::endl;
    std::cout << "Messages must be no more than 1000 characters long. For spaces use '_'." << std::endl;

    ThreadParams *threadParam = new ThreadParams{msgIdFirst, msgIdSecond};
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

    msgctl(msgIdFirst, IPC_RMID, NULL);
    msgctl(msgIdSecond, IPC_RMID, NULL);

    return 0;
}