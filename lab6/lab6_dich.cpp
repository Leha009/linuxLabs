#include <stdlib.h>
#include <iostream>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <ctime>
#include <unistd.h>
#include <sys/wait.h>

clock_t childStart, childStop;

void SignalHandler(int iSignal)
{
    if(iSignal == SIGALRM)
    {
        pid_t pid = fork();
        if(pid == 0)
        {
            childStart = clock();
            std::time_t time = std::time(NULL);
            std::cout << std::string(70, '_').c_str() << std::endl 
                      << "Child: my pid is " << getpid() << ". Current date: "
                      << std::ctime(&time);     // Там уже есть перенос строки
            std::cout << "C: " << childStart << std::endl;
            childStop = clock();
            std::cout << childStop-childStart << " clocks\n";
            exit(EXIT_SUCCESS);
        }
        else if(pid == -1)
        {
            std::cout << "Failed to create child :c" << std::endl;
            exit(EXIT_FAILURE);
        }

        waitpid(pid, NULL, 0);

        std::cout << "P: " << childStart << std::endl;
        
        
        std::cout << "Child worked for " << (float)(childStop - childStart)/(CLOCKS_PER_SEC/1000) << " ms" << std::endl;
    }
}

int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        std::cout << "Syntax to run programm: ./lab6 {period in seconds} {times}" << std::endl;
        return 0;
    }

    int iPeriod = atoi(argv[1]);
    int iTimes = atoi(argv[2]);

    if(iPeriod < 1)
    {
        std::cout << "Period must be positive!" << std::endl;
        return 0;
    }

    if(iTimes < 1)
    {
        std::cout << "Times must be positive!" << std::endl;
        return 0;
    }

    signal(SIGALRM, SignalHandler);
    signal(SIGTSTP, SIG_IGN);

    struct itimerval tv;
    tv.it_interval.tv_sec = iPeriod;    // след
    tv.it_interval.tv_usec = 0;         
    tv.it_value.tv_sec = iPeriod;       // тек
    tv.it_value.tv_usec = 0;

    if(setitimer(ITIMER_REAL, &tv, NULL))
    {
        perror("Failed to create timer");
        return errno;
    }

    time_t start, stop;

    std::cout << std::string(33, '+') << "START" << std::string(32, '+') << std::endl;

    start = time(NULL);

    for(int i = 0; i < iTimes; ++i)
        pause();

    stop = time(NULL);
    std::cout << "Parent worked for " << (stop - start) << " s" << std::endl << std::string(70, '=').c_str() << std::endl;

    return 0;
}