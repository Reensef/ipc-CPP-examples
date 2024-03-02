#include <stdio.h>
#include <error.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/sem.h>

#include <iostream>
#include <string>
#include <cstring>

#define PERMISSION_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
// More about permission mode flag:
// https://www.gnu.org/software/libc/manual/html_node/Permission-Bits.html

int main(int argc, char **argv)
{
    // IPC_CREAT - Create entry if key does not exist
    // More: https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_ipc.h.html
    int oflag = PERMISSION_MODE | IPC_CREAT;

    // getopt - Get options from cmd
    // More: https://man7.org/linux/man-pages/man3/getopt.3.html
    if (argc != 2)
    {
        printf("usage: client <path_to_file");
        return 0;
    }

    key_t semKey = ftok(argv[1], 'S');   // Generate semaphore key;
    int semId = semget(semKey, 1, 0666); // Semaphore creation

    if (semId == -1)
    {
        std::cerr << "Semaphore creation error" << std::endl;
        return 1;
    }

    struct sembuf semBuf;
    semBuf.sem_num = 0;
    semBuf.sem_op = 0;
    semBuf.sem_flg = 0;

    // Creation segment

    // ftok - Convert a pathname and a project identifier to a System V IPC key
    // More: https://man7.org/linux/man-pages/man3/ftok.3.html

    // shmget - Get shared memory segment
    // More: https://man7.org/linux/man-pages/man2/shmget.2.html

    // shmat - Connects a segment to the address space process
    // More: https://man.freebsd.org/cgi/man.cgi?query=shmat&sektion=2&n=1
    key_t segKey = ftok(argv[1], 'R');
    int segId = shmget(segKey, 1000, oflag);

    if (segId == -1)
    {
        std::cout << "Error accessing shared memory segment" << std::endl;
        return 1;
    }

    char *ptr = (char *)shmat(segId, NULL, 0);

    std::cout << "You are ready to receive messages." << std::endl;

    // Read from segment
    while (true)
    {
        semBuf.sem_op = -1; // Wait
        if (semop(semId, &semBuf, 1) == -1)
        {
            perror("semop");
            exit(1);
        }

        ptr = (char *)shmat(segId, NULL, 0);
        std::cout << ptr << std::endl;
    }
    
    shmdt(ptr);
    semctl(semId, 0, IPC_RMID, NULL);
    semctl(segId, 0, IPC_RMID, NULL);

    return 0;
}