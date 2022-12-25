//1я задача server
#include <iostream>
#include <unistd.h>
#include <sys/msg.h>
#include <signal.h>
#include <sys/time.h>

#define KEY 276000

int msg_queue;

void end_func(int sig)
{
    msgctl(msg_queue, IPC_RMID, 0);
    std::cout << "I'm done with it!" << std::endl;
    exit(EXIT_SUCCESS);
}

struct message
{
    long id;
    int new_time;
};

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        std::cout << "Syntax error! ./server {timeout in seconds}" << std::endl;
        return 0;
    }

    int timeout = atoi(argv[1]);
    if(timeout < 1)
    {
        std::cout << "Timeout must be positive!" << std::endl;
        return 0;
    }

    msg_queue = msgget(KEY, 0666 | IPC_CREAT);
    if(msg_queue == -1)
    {
        perror("Failed to create message queue");
        return errno;
    }

    std::cout << "I will wait for " << timeout << " seconds..." << std::endl;
    
    signal(SIGALRM, end_func);

    itimerval tv;
    tv.it_interval.tv_sec = timeout;    // след
    tv.it_interval.tv_usec = 0;         
    tv.it_value.tv_sec = timeout;       // тек
    tv.it_value.tv_usec = 0;

    if(setitimer(ITIMER_REAL, &tv, NULL))
    {
        perror("Failed to create timer");
        return errno;
    }

    message msg;
    while(1)
    {
        if(msgrcv(msg_queue, &msg, sizeof(int), 0, 0) != -1)
        {
            std::cout << "New timeout (got from " << msg.id << " id) is " << msg.new_time << std::endl;
            timeout = msg.new_time;
            tv.it_interval.tv_sec = timeout;    // след
            tv.it_value.tv_sec = timeout;       // тек
            setitimer(ITIMER_REAL, &tv, NULL);
        }
    }

    return 0;
}