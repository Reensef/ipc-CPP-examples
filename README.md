## Named Pipes
In this example, **2 executable** files are compiled. Each has **one process** and **2 threads**, one for reading and the other for writing. 
The processes are connected through two channels.
- There is **no need** to create a file for the pipe, as it can do it on its own.

## Unnamed Pipes
In this example, a **single** executable file is compiled. The process branches into two processes. One reads the data, the other writes. The interaction takes place through an not named piped.
- Unnamed pipes do not need to be linked to a file
- A parent process is required to create the use of unnamed pipes
  
## Shared Memory
In this example, a **single** executable file is compiled. The executable file can be run an unlimited number of times (conditionally). A **publish-subscribe template** has been implemented, in which each process can receive messages from everyone and send messages to everyone. The interaction takes place through **shared memory**.
- To create a shared memory, you need to create a file.
- The example uses a semaphore, it can use a shared memory file, but with a different proj_id.

## Message Queue
In this example, 2 executable files are compiled. Each has one process and 2 threads, one for reading and the other for writing. Interactions occur through message queues.

- To create a message queue, you need to create a file.