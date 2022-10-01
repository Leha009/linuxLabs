#include <iostream>
#include <pthread.h>
#include <fstream>
#include <unistd.h>

void* ReadByThread(void*);

bool bCloseByThread = false;

int main(int argc, char* argv[])
{
    if(argc == 2)
    {
        bCloseByThread = (bool)atoi(argv[1]);
    }

    std::fstream inFile;
    inFile.open("textFile.txt", std::ios::in);

    if(!inFile.is_open())
    {
        perror("Failed to open the file");
        return errno;
    }

    pthread_t thr;
    int result = pthread_create(&thr, NULL, ReadByThread, &inFile);

    pthread_join(thr, NULL);

    sched_param schedParam;
    sched_getparam(getpid(), &schedParam);

    std::cout << "My (program) priority: " << schedParam.sched_priority << std::endl;

    if(inFile.is_open())
    {
        std::cout << "File wasn't closed. Closing it..." << std::endl;
        inFile.close();
    }
    else
        std::cout << "File was closed by thread" << std::endl;

    return 0;
}

void* ReadByThread(void* param)
{
    //Получение класа планирования
    //SCHED_FIFO = 1, SCHED_RR = 2, SCHED_OTHER = 0
    int iClass = sched_getscheduler(getpid());

    sched_param schedParam;
    sched_getparam(getpid(), &schedParam);

    /*
    В Linux допускаются статические приоритеты в диапазоне от 1 до 99 
    для политик SCHED_FIFO и SCHED_RR и приоритет 0 для для остальных политик
    */
    int iMaxPriority = sched_get_priority_max(iClass);
    int iMinPriority = sched_get_priority_min(iClass);

    std::cout << "Sched class: " << iClass << std::endl;
    std::cout << "My (thread) priority: " << schedParam.sched_priority 
              << ". Min: " << iMinPriority << ", max: " << iMaxPriority
              << std::endl;

    std::fstream* pFile = (std::fstream*)param;
    std::string sBuffer;
    std::cout << "Here is text in file:" << std::endl;
    while(getline(*pFile, sBuffer))
    {
        std::cout << sBuffer << std::endl;
    }

    if(bCloseByThread)
        pFile->close();
    return NULL;
}