#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>

#define SEQ_LEN 10

void service_client(int accepted_socket, int socket, int timeout);

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        std::cout << "Syntax error! ./server {timeout in seconds}" << std::endl;
        return 0;
    }
    int timeout = atoi(argv[1]);

    timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        perror("Failed to create socket");
        return errno;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;  //0,0,0,0
    addr.sin_port = htons(1505);    // в сетевые байты

    if(bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("Failed to bind");
        return errno;
    }

    listen(sock, 5);    //Очередь из 5
    std::cout << "Listening..." << std::endl;

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);

    while(1)
    {
        //https://manpages.courier-mta.org/htmlman2/select.2.html
        int status = select(sock+1, &readfds, NULL, NULL, &tv);
        if(status > 0)
        {
            sockaddr_in client;
            int accepted = accept(sock, (sockaddr*)&client, (socklen_t*)&client);
            if(accepted > 0)
            {
                std::cout << "Accepted client" << std::endl;
                if(fork() == 0)
                    service_client(accepted, sock, timeout);
                tv.tv_sec = timeout;
                tv.tv_usec = 0;
            }
        }
        else
        {
            std::cout << "TIMEOUT!" << std::endl;
            break;
        }
    }

    close(sock);
    return 0;
}

void service_client(int accepted_socket, int socket, int timeout)
{
    close(socket);
    int num_seq[SEQ_LEN], read_bytes;

    timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    setsockopt(accepted_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    if(recv(accepted_socket, num_seq, 4*SEQ_LEN, 0) > 0)
    {
        std::cout << "Got sequence: " << num_seq[0];
        for(int i = 1; i < SEQ_LEN; ++i)
            std::cout << ',' << num_seq[i];
        std::cout << std::endl;

        sort(num_seq, num_seq+SEQ_LEN, std::greater<int>());
        std::cout << "Sorted sequence like that: ";
        for(int i = 0; i < SEQ_LEN; ++i)
        {
            if(i)
                std::cout << ",";

            std::cout << num_seq[i];
        }
        std::cout << std::endl;

        send(accepted_socket, num_seq, 4*SEQ_LEN, 0);

        std::cout << getpid() << ": sent answer to client" << std::endl << std::endl;
    }

    close(accepted_socket);
    exit(0);
}