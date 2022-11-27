#include <iostream>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fstream>
#include <time.h>

#define SHM_KEY 6117
#define FILE_NAME "out.txt"
#define PROG_MAIN 0

struct shared
{
    bool choosing[3];
    int number[3];
};

int _shmget(key_t key, int iProgNum);
void mount_shm(int iShmId, shared** sharedData, int iProgNum);
void unmount_shm(shared* sharedData, int iProgNum);
void destroy_shm(int iShmId, int iProgNum);

int cmp_tuple(int a, int b, int c, int d);
int max(int* iMassive, size_t size);

void write_to_file(std::ofstream& outFile, int iProgNum);
void write_action(int iProgNum, const char* sAction);

std::string gen_random_text(const int len);

int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        std::cout << "./main {prog number} {writing count} {delay}" << std::endl;
        return 0;
    }

    int iProgNumber = atoi(argv[1]);
    int iWritingCount = atoi(argv[2]);
    int iDelay = atoi(argv[3]);

    if(iProgNumber < 0 || iProgNumber > 2)
    {
        std::cout << "Program number must be 0 or 1 or 2!" << std::endl;
        return 0;
    }

    if(iWritingCount < 1)
    {
        std::cout << "Number of times to write to files must be positive!" << std::endl;
        return 0;
    }

    if(iDelay < 1)
    {
        std::cout << "Delay must be positive!" << std::endl;
        return 0;
    }

    srandom(time(NULL));

    int iShmId = _shmget(SHM_KEY, iProgNumber);
    shared* shData;
    std::string numToConsole;

    mount_shm(iShmId, &shData, iProgNumber);

    for(int i = 0; i < iWritingCount; ++i)
    {
        shData->choosing[iProgNumber] = true;
        shData->number[iProgNumber] = max(shData->number, 3)+1;
        shData->choosing[iProgNumber] = false;

        numToConsole = "num is " + std::to_string(shData->number[iProgNumber]) + "\n";
        write_action(iProgNumber, numToConsole.c_str());

        for(int j = 0; j < 3; ++j)
        {
            if(j == iProgNumber)
                continue;

            while(shData->choosing[j]);

            while(shData->number[j] != 0 && 
                    cmp_tuple(shData->number[j], j, 
                    shData->number[iProgNumber], iProgNumber) < 0);

        }

        std::ofstream outFile(FILE_NAME, std::ios::app);
        write_to_file(outFile, iProgNumber);
        outFile.close();
        shData->number[iProgNumber] = 0;
        sleep(iDelay);
    }

    unmount_shm(shData, iProgNumber);
    if(iProgNumber == PROG_MAIN)
        destroy_shm(iShmId, iProgNumber);

    write_action(iProgNumber, "done!\n");
    return 0;
}

int _shmget(key_t key, int iProgNum)
{
    int iShmId = -1;

    if(iProgNum == 0)
    {
        iShmId = shmget(key, sizeof(shared), 0666 | IPC_CREAT);
        write_action(iProgNum, "create shared segment\n");
    }
    else
    {
        do
        {
            iShmId = shmget(key, sizeof(shared), IPC_EXCL);
        } while(iShmId == -1);

        write_action(iProgNum, "got shared segment\n");
    }

    return iShmId;
}

void mount_shm(int iShmId, shared** sharedData, int iProgNum)
{
    *sharedData = (shared*)shmat(iShmId, NULL, 0);
    write_action(iProgNum, "mounted shared param\n");
}

void unmount_shm(shared* sharedData, int iProgNum)
{
    if(shmdt(sharedData) == -1)
    {
        perror("failed to unmount");
        exit(errno);
    }
    write_action(iProgNum, "unmounted shared param\n");
}

void destroy_shm(int iShmId, int iProgNum)
{
    if(shmctl(iShmId, IPC_RMID, NULL) == -1)
    {
        write_action(iProgNum, "failed to destroy shm!\n");
        perror("failed to destroy shm");
        exit(EXIT_FAILURE);
    }
    
    write_action(iProgNum, "destroyed shm!\n");
}

void write_to_file(std::ofstream& outFile, int iProgNum)
{
    std::string sToFile;
    write_action(iProgNum, "started writing...\n");

    //sToFile = gen_random_text(random() % 6 + 1) + "\n";
    sToFile = std::string(6, (char) (iProgNum+48)) + "\n";
    outFile << sToFile;

    sToFile = "finished to write string: " + sToFile;
    write_action(iProgNum, sToFile.c_str());
}

int max(int* iMassive, size_t size)
{
    int iMax = 0;
    for(size_t i = 0UL; i < size; i++)
    {
        if(iMassive[i] > iMax)
            iMax = iMassive[i];
    }

    return iMax;
}

int cmp_tuple(int a, int b, int c, int d)
{
    if(a != c)
        return a-c;

    return b-d;
}

void write_action(int iProgNum, const char* sAction)
{
    clock_t cTime = clock();
    std::cout << cTime << " | Prog #" << iProgNum << ": " << sAction;
}

std::string gen_random_text(const int len)
{
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    
    return tmp_s;
}