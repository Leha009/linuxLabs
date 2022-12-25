//1я задача client
#include <iostream>
#include <unistd.h>
#include <sys/msg.h>

#define KEY 276000

struct message
{
    long id;
    int new_time;
};

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        std::cout << "Syntax error! ./client {new timeout in seconds}" << std::endl;
        return 0;
    }

    int new_time = atoi(argv[1]);

    if(new_time < 1)
    {
        std::cout << "New time must be positive!" << std::endl;
        return 0;
    }

    int msg_queue = msgget(KEY, IPC_EXCL);
    if(msg_queue == -1)
    {
        std::cout << "Server is down now!" << std::endl;
        msgctl(msg_queue, IPC_RMID, 0);
        return 0;
    }

    message msg;
    msg.id = getpid();
    msg.new_time = new_time;

    if(msgsnd(msg_queue, &msg, sizeof(int), 0) == -1)
    {
        perror("Failed to send new time");
        return errno;
    }
    
    std::cout << "Your id is " << getpid() << std::endl;
    std::cout << "Server got your new timeout: " << new_time << std::endl;

    return 0;
}