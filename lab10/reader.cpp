#include <iostream>
#include <fstream>
#include <sys/sem.h>

#define _semop(_sembuf) semop(sem_id, &_sembuf, 1)

#define DEBUG 0
#define LOG(str) if(DEBUG) \
                    std::cout << str << std::endl

#define FILE_NAME "file"
#define SEM_KEY 276001

#define READERS 0
#define WRITERS 1
#define BUSYFILE 2
#define RUNPROGS 3

int main(int argc, char* argv[])
{
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
        LOG("Reader: FILE++");
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
    LOG("Reader: RUN++");

    sbuf = {WRITERS, 0, 0};   // Проверка наличия работающих писателей
    _semop(sbuf);
    LOG("Reader: WRITERS??");

    sbuf = {READERS, 1, 0};   // Читатели++
    _semop(sbuf);
    LOG("Reader: READERS++");

    std::ifstream in_file(FILE_NAME);
    std::string str;
    while(getline(in_file, str))
    {
        std::cout << str << std::endl;
    }

    in_file.close();

    sbuf = {READERS, -1, 0};   // Читатели--
    _semop(sbuf);
    LOG("Reader: READERS--");

    sbuf = {RUNPROGS, -1, 0};  // Рабочие программы--
    _semop(sbuf);
    LOG("Reader: RUN--");

    if(sem_creator)
    {
        sbuf = {RUNPROGS, 0, 0};
        _semop(sbuf);
        semctl(sem_id, IPC_RMID, 0);    // Удаляет семафоры
    }
    return 0;
}