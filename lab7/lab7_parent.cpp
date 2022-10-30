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

    int pipes[2];  //0 - read, 1 - write
    int iPipeStatus = pipe2(pipes, O_NONBLOCK);
    if(iPipeStatus == -1)
    {
        perror("Cannot create pipe");
        return errno;
    }

    signal(SIGUSR1, SIG_IGN);
    signal(SIGUSR2, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);


    pid_t child1 = vfork();
    if(child1 == -1)
    {
        perror("Failed to create child1");
        return errno;
    }
    else if(child1 == 0)
    {
        std::string pipeStr = std::to_string(pipes[0]);
        execl("child1", "child1", pipeStr.c_str(), NULL);
        return 0;
    }

    pid_t child2 = vfork();
    if(child2 == -1)
    {
        perror("Failed to create child2");
        return errno;
    }
    else if(child2 == 0)
    {
        std::string pipeStr = std::to_string(pipes[0]);
        execl("child2", "child2", pipeStr.c_str(), NULL);
        return 0;
    }

    std::ifstream inFile(sFileName);
    ReadFileToPipe(inFile, pipes[1], child1);
    kill(child1, SIGQUIT);
    kill(child2, SIGQUIT);

    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);

    inFile.close();
    close(pipes[0]);
    close(pipes[1]);

    return 0;
}

void ReadFileToPipe(std::ifstream& inFile, int iPipe, int iFirstChild)
{
    sleep(1);
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
