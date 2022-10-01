#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sstream>

void LogToFile(const char* sProccessName, int fd);

int main(int argc, char* argv[])
{
    char* sPath = argv[1];
    unsigned iDelay = (unsigned)atoi(argv[2]);

    //Open file with create, write and append flags. Mode is READ/WRITE
    int fd = open(sPath, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);

    if(fd == -1)
    {
        perror("Can't open the file");
        return errno;
    }

    sleep(iDelay);

    LogToFile("Vfork", fd);

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
    ss << "pid is " << pid << ", ppid is " << ppid << ", ";
    ss << "uid is " << uid << ", euid is " << euid << ", ";
    ss << "gid is " << gid << ", egid is " << egid << std::endl;

    std::string sOutput = ss.str();
    write(fd, sOutput.c_str(), sOutput.size());
}