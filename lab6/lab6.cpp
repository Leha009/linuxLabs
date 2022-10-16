#include <stdlib.h>
#include <iostream>
#include <signal.h>
#include <sys/time.h>
#include <ctime>
#include <unistd.h>
#include <sys/wait.h>
#include <chrono>

void SignalHandler(int iSignal)
{
    if(iSignal == SIGALRM)
    {
        std::chrono::_V2::system_clock::time_point cWTStart, cWTStop;
        pid_t pid = fork();
        if(pid == 0)
        {
            std::time_t time = std::time(NULL);
            std::cout << std::string(70, '_').c_str() << std::endl 
                      << "Child: my pid is " << getpid() << ". Current date: "
                      << std::ctime(&time);     // Там уже есть перенос строки
            exit(EXIT_SUCCESS);
        }
        else if(pid == -1)
        {
            std::cout << "Failed to create child :c" << std::endl;
            exit(EXIT_FAILURE);
        }

        cWTStart = std::chrono::high_resolution_clock::now();
        waitpid(pid, NULL, 0);
        cWTStop = std::chrono::high_resolution_clock::now();
        std::chrono::microseconds usec = std::chrono::duration_cast<std::chrono::microseconds>(cWTStop - cWTStart);
        std::cout << "Child worked for " << usec.count() << " micros" << std::endl;
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

    std::chrono::_V2::system_clock::time_point pWTStart, pWTimeStop;

    std::cout << std::string(33, '+') << "START" << std::string(32, '+') << std::endl;

    pWTStart = std::chrono::high_resolution_clock::now();

    for(int i = 0; i < iTimes; ++i)
        pause();

    pWTimeStop = std::chrono::high_resolution_clock::now();
    auto usec = std::chrono::duration_cast<std::chrono::microseconds>(pWTimeStop - pWTStart);
    std::cout << "Parent worked for " << usec.count() << " micros" << std::endl << std::string(70, '=').c_str() << std::endl;

    return 0;
}