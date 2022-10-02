#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sstream>

void LogToFile(const char* sProccessName, int fd);

int main(int argc, char* argv[])
{
    if(argc != 5)
    {
        std::cout << "You forgot parameters: {file path} {main program delay} {fork delay} {vfork delay}" << std::endl;
        return 0;
    }

    char* sFilePath = argv[1];
    unsigned iMainDelay = (unsigned)atoi(argv[2]);
    unsigned iForkDelay = (unsigned)atoi(argv[3]);
    char* sVforkDelay = argv[4];

    //Open file with create, write and append flags.
    int fd = open(sFilePath, O_CREAT | O_WRONLY | O_APPEND);

    if(fd == -1)
    {
        perror("Can't open the file");
        return errno;
    }

    pid_t pidFork = fork();
    if(pidFork == -1)
    {
        perror("Fork failed");
        return errno;
    }
    else if(pidFork == 0)   //For new only
    {
        sleep(iForkDelay);
        LogToFile("Fork", fd);
        return 0;     // To stop fork doing stuff below
    }

    pid_t pidVfork = vfork();
    if(pidVfork == -1)
    {
        perror("VFork failed");
        return errno;
    }
    else if(pidVfork == 0)
    {
        execl("vfork", "vfork", sFilePath, sVforkDelay, NULL);
    }

    sleep(iMainDelay);

    LogToFile("Main", fd);

    close(fd);

    return 0;
}

void LogToFile(const char* sProccessName, int fd)
{
    pid_t   pid = getpid(),     //Process id
            sid = getsid(pid),  //Process session  id
            ppid = getppid();   //Parent process id
    uid_t   uid = getuid(),     //User id
            euid = geteuid();   //Effective user id
    gid_t   gid = getgid(),     //Group id
            egid = getegid();   //Effective group id

    std::stringstream ss;
    ss << sProccessName << ": ";
    ss << "pid:" << pid << ", ppid:" << ppid << ", ";
    ss << "uid:" << uid << ", euid:" << euid << ", ";
    ss << "gid:" << gid << ", egid:" << egid;
    ss << ", sid:" << sid << std::endl;

    std::string sOutput = ss.str();
    write(fd, sOutput.c_str(), sOutput.size());
}