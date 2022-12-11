#include <iostream>
#include <fstream>
#include <sys/sem.h>
#include <unistd.h>
#include <ctime>

#define _semop(_sembuf) semop(sem_id, &_sembuf, 1)

#define DEBUG 0
#define LOG(str) if(DEBUG) \
                    std::cout << str << std::endl

#define FILE_NAME "file"
#define SEM_KEY 276001  //1 -> 0?

#define READERS 0
#define WRITERS 1
#define BUSYFILE 2
#define RUNPROGS 3

int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        std::cout << "Syntax error! ./writer {num of writing in file} {sleep time}" << std::endl;
        return 0;
    }

    int repeats = atoi(argv[1]);
    if(repeats < 1)
    {
        std::cout << "Num of writing in file must be positive!" << std::endl;
        return 0;
    }

    int sleep_time = atoi(argv[2]);
    if(sleep_time < 1)
    {
        std::cout << "Sleep time must be positive!" << std::endl;
        return 0;
    }

    bool sem_creator;   // Этот ли процесс создал все семафоры
    /*
    sembuf.sem_num - индекс семафора в массиве 
    sembuf.sem_op - операция
    sembuf.sem_flg - флаги:
    
                        о IPC_NOWAIT - если не успешна операция,
                                то -1 и значение не меняем
                        о SEM_UNDO - при завершении этого
                                процесса ядро даст обратный ход всем операциям,
                                выполненным процессом
    */
    struct sembuf sbuf;

    /*
    Нужно на:
        0 - читатели
        1 - писатели
        2 - занят ли файл
        3 - на завершение всех программ
    */
    key_t sem_id = semget(SEM_KEY, 4, 0666 | IPC_CREAT | IPC_EXCL);
    if(sem_id != -1)
    {
        sem_creator = true;
        sbuf = {BUSYFILE, 1, 0};    // Делаем семафор на файл
        _semop(sbuf);
        LOG("Writer: FILE++");
    }
    else
        sem_id = semget(SEM_KEY, 4, 0666 | IPC_CREAT);

    if(sem_id < 0)
    {
        perror("Failed to create semaphore");
        return errno;
    }

    sbuf = {RUNPROGS, 1, 0};   // Текущие программы++
    _semop(sbuf);
    LOG("Writer: RUN++");

    std::string str;

    for(int i = 0; i < repeats; ++i)
    {
        sbuf = {WRITERS, 1, 0}; // Писатели++
        _semop(sbuf);
        LOG("Writer: WRITERS++");

        sbuf = {READERS, 0, 0}; // Смотрим, закончили ли читатели
        _semop(sbuf);
        LOG("Writer: READERS??");

        sbuf = {BUSYFILE, -1, 0}; // Занимаем файл для записи
        _semop(sbuf);
        LOG("Writer: FILE--");

        //sleep(1);   // Видимость долгой записи :)

        std::time_t time = std::time(NULL);
        std::ofstream out_file(FILE_NAME, std::ios::app);
        str = "Writer" + std::to_string(getpid()) + ": " + std::ctime(&time);
        out_file << str;
        std::cout << str;
        out_file.close();

        sbuf = {BUSYFILE, 1, 0}; // Освобождаем файл
        _semop(sbuf);
        LOG("Writer: FILE++");

        sbuf = {WRITERS, -1, 0}; // Говорим, что этот писатель закончил работу
        _semop(sbuf);
        LOG("Writer: WRITERS--");

        sleep(sleep_time);
    }

    sbuf = {RUNPROGS, -1, 0};  // Рабочие программы--
    _semop(sbuf);
    LOG("Writer: RUN--");

    if(sem_creator)
    {
        sbuf = {RUNPROGS, 0, 0};
        _semop(sbuf);
        semctl(sem_id, IPC_RMID, 0);    // Удаляет семафоры
    }
    return 0;
}