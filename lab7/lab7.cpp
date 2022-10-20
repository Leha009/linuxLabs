#include <iostream>
#include <fstream>
#include <sys/signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

void ReadFileToPipe(std::ifstream& inFile, int iPipe, int iFirstChild);
void WriteFileChild(int iPipe, int iChild);
void SignalHandler(int iSignal);

bool g_bEnd, g_bRead;

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        std::cout << "You forgot param: ./lab7 {file to read}" << std::endl;
        return 0;
    }

    char* sFileName = argv[1];

    int fields[2];  //0 - read, 1 - write
    int iPipe = pipe2(fields, O_NONBLOCK);

    signal(SIGUSR1, SIG_IGN);
    signal(SIGUSR2, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);

    pid_t child1 = fork();
    if(child1 == -1)
    {
        perror("Failed to create child1");
        return errno;
    }
    else if(child1 == 0)
    {
        signal(SIGUSR1, SignalHandler);
        signal(SIGQUIT, SignalHandler);
        signal(SIGUSR2, SIG_IGN);
        //close(fields[1]);
        WriteFileChild(fields[0], 1);
        return 0;
    }

    pid_t child2 = fork();
    if(child2 == -1)
    {
        perror("Failed to create child2");
        return errno;
    }
    else if(child2 == 0)
    {
        signal(SIGUSR2, SignalHandler);
        signal(SIGQUIT, SignalHandler);
        signal(SIGUSR1, SIG_IGN);
        //close(fields[1]);
        WriteFileChild(fields[0], 2);
        return 0;
    }

    std::ifstream inFile(sFileName);
    ReadFileToPipe(inFile, fields[1], child1);
    kill(child1, SIGQUIT);
    kill(child2, SIGQUIT);

    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);

    inFile.close();
    close(fields[0]);
    close(fields[1]);

    return 0;
}

void ReadFileToPipe(std::ifstream& inFile, int iPipe, int iFirstChild)
{
    bool bFirstSignal;
    if(!inFile.is_open())
    {
        perror("Failed to read file");
        exit(errno);
    }

    std::string sBuffer;
    while(getline(inFile, sBuffer))
    {
        sBuffer += "\n";
        //std::cout << "write to pipe: " << sBuffer << std::endl;
        write(iPipe, sBuffer.c_str(), sBuffer.size());
        if(!bFirstSignal)
        {
            bFirstSignal = true;
            kill(iFirstChild, SIGUSR1);
        }
    }
    if(!bFirstSignal)
        kill(iFirstChild, SIGUSR1);
}

void SignalHandler(int iSignal)
{
    if(iSignal == SIGQUIT)
        g_bEnd = true;
    else
        g_bRead = true;

    //std::cout << "\n" << iSignal << "!!!got signal!!!\n";
}

void WriteFileChild(int iPipe, int iChild)
{
    std::string sFileName = "child" + std::to_string(iChild);
    std::ofstream outFile(sFileName.c_str());
    //std::cout << iChild << ": ready. Pipe is " << iPipe << "\n";
    int iMySignal = (iChild == 1 ? SIGUSR1 : SIGUSR2),
        iOtherSignal = (iChild == 1 ? SIGUSR2 : SIGUSR1);
    ssize_t iReadBytes;
    bool bEnd;
    char cBuffer;
    while(!bEnd)
    {
        //std::cout << iChild << ": wait for " << iMySignal << "\n";
        while(!g_bRead)
            pause();
        g_bRead = false;

        //std::cout << iChild << " got signal. Reading from pipe...\n";
        iReadBytes = read(iPipe, &cBuffer, sizeof(cBuffer));
        if(iReadBytes > 0L)
        {
            //std::cout << iChild << " got (" << iReadBytes << "): " << cBuffer << std::endl;
            outFile << cBuffer;
        }
        else
        {
            //perror("Failed to read pipe");
            bEnd = g_bEnd;
        }

        //std::cout << iChild << " finished reading from pipe, raise signal...\n";
        kill(0, iOtherSignal);
    }
    //std::cout << iChild << ": out\n";
    outFile.close();
}