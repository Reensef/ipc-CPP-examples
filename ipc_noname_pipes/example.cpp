#include <iostream>
#include <unistd.h>

#include <fcntl.h>

int main()
{
    int descriptors[2];
    pipe(descriptors);

    pid_t pid = fork();

    if (pid < 0)
    {
        std::cerr << "Error process creation" << std::endl;
        return 1;
    }
    else if (pid > 0)
    {
        pid_t pid = getpid();

        while (true)
        {
            char message[] = "Data from process one";

            std::cout << pid << " send: " << message << std::endl;

            write(descriptors[1], message, sizeof(message));
        }

        close(descriptors[1]); // never run, but use in different situation
    }
    else
    {
        pid_t pid = getpid();

        while (true)
        {
            char recvMessage[1000];

            read(descriptors[0], recvMessage, sizeof(recvMessage));

            std::cout << pid << " recived: " << recvMessage << std::endl;
        }

        close(descriptors[0]); // never run, but use in different situation
    }

    return 1;
}
