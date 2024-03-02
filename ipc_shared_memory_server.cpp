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

    int c;
    // getopt - Get options from cmd
    // More: https://man7.org/linux/man-pages/man3/getopt.3.html
    while ((c = getopt(argc, argv, "e")) != -1)
    {
        switch (c)
        {
        // Флаг IPC_EXCL необходим, чтобы предотвратить ситуацию, когда процесс
        // полагает, что получил новый (уникальный) идентификатор очереди сообщений,
        // хотя это не так. Иными словами, когда используются и IPC_CREAT и IPC_EXCL,
        // при успешном завершении системного вызова обязательно возвращается новый
        // идентификатор
        case 'e':
            // IPC_EXCL - Fail if key exists.
            oflag |= IPC_EXCL;
            break;
        }
    }

    if (optind != argc - 1)
    {
        printf("usage: server [ -e 'Fail if key exists' ] <path_to_file>");
        return 0;
    }

    key_t semKey = ftok(argv[1], 'S');               // Generate semaphore key
    int semId = semget(semKey, 1, 0666 | IPC_CREAT); // Semaphore creation

    if (semId == -1)
    {
        std::cerr << "Semaphore creation error" << std::endl;
        return 1;
    }

    struct sembuf semBuf;
    semBuf.sem_num = 0;
    semBuf.sem_op = 0;
    semBuf.sem_flg = SEM_UNDO;

    // Creation segment

    // ftok - Convert a pathname and a project identifier to a System V IPC key
    // More: https://man7.org/linux/man-pages/man3/ftok.3.html

    // shmget - Get shared memory segment
    // More: https://man7.org/linux/man-pages/man2/shmget.2.html

    // shmat - Connects a segment to the address space process
    // More: https://man.freebsd.org/cgi/man.cgi?query=shmat&sektion=2&n=1

    // proj_id ('R') - used to ensure key uniqueness within one project or application.
    key_t segKey = ftok(argv[optind], 'R');
    int segId = shmget(segKey, 1000, oflag);
    char *ptr = (char *)shmat(segId, NULL, 0);

    // shmctl - System V shared memory control
    // More: https://man7.org/linux/man-pages/man2/shmctl.2.html

    // Read user data
    std::cout << "Enter a message of no more than 1000 characters" << std::endl;
    while (true)
    {
        char inputString[1000];
        std::cin >> inputString;
        // Write to segement
        strcpy(ptr, inputString);

        semBuf.sem_op = 1; // Open to read
        if (semop(semId, &semBuf, 1) == -1)
        {
            perror("semop");
            exit(1);
        }
    }
    shmdt(ptr);
    semctl(semId, 0, IPC_RMID, NULL);
    semctl(segId, 0, IPC_RMID, NULL);

    return 0;
}