#include <iostream>
#include <fstream>
#include <sys/signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

void WriteFileChild(int iPipe, int iChild);
void SignalHandler(int iSignal);

bool g_bEnd, g_bRead;

int main(int argc, char* argv[])
{
    if(argc < 2)
        return 1;

    int iReadPipe = atoi(argv[1]);
    signal(SIGUSR2, SignalHandler);
    signal(SIGQUIT, SignalHandler);
    WriteFileChild(iReadPipe, 2);

    return 0;
}

void SignalHandler(int iSignal)
{
    if(iSignal == SIGQUIT)
        g_bEnd = true;
    else
        g_bRead = true;
}

void WriteFileChild(int iPipe, int iChild)
{
    std::string sFileName = "child" + std::to_string(iChild) + ".txt";
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
