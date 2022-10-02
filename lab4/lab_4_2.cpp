#include <iostream>
#include <pthread.h>
#include <fstream>

struct threadStruct
{
    std::ofstream* pOutFile;
    std::string stringToWrite;
};

void* ThreadWrite(void*);

int main(int argc, char* argv[])
{
    std::ifstream inFile;
    inFile.open("textFile.txt", std::ios::in);

    if(!inFile.is_open())
    {
        perror("Failed to open file for reading");
        return errno;
    }

    std::ofstream outFile1, outFile2;
    outFile1.open("out1.txt", std::ios::out);
    if(!outFile1.is_open())
    {
        inFile.close();
        perror("Failed to open 1st file for writing");
        return errno;
    }

    outFile2.open("out2.txt", std::ios::out);

    if(!outFile2.is_open())
    {
        inFile.close();
        outFile1.close();
        perror("Failed to open 2nd file for writing");
        return errno;
    }

    std::string sBuffer1, sBuffer2;
    pthread_t thr1 = 0, thr2 = 0;
    threadStruct ts1, ts2;
    ts1.pOutFile = &outFile1;
    ts2.pOutFile = &outFile2;

    while(!inFile.eof())
    {
        if(getline(inFile, sBuffer1))
        {
            ts1.stringToWrite = sBuffer1;
            if(pthread_create(&thr1, NULL, ThreadWrite, &ts1))
            {
                perror("Error in thread1");
                return errno;
            }
        }

        if(!inFile.eof() && getline(inFile, sBuffer2))
        {
            ts2.stringToWrite = sBuffer2;
            if(pthread_create(&thr2, NULL, ThreadWrite, &ts2))
            {
                perror("Error in thread2");
                return errno;
            }
        }

        if(thr1)
            pthread_join(thr1, NULL);
        if(thr2)
            pthread_join(thr2, NULL);
    }

    inFile.close();
    outFile1.close();
    outFile2.close();
}

void* ThreadWrite(void* param)
{
    threadStruct* pts = (threadStruct*)param;
    std::ofstream& outFile = *(pts->pOutFile);
    outFile << pts->stringToWrite << std::endl;
    return NULL;
}