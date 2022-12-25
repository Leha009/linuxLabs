#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#define SEQ_LEN 10

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        std::cout << "Syntax error! ./client {timeout in seconds}" << std::endl;
        return 0;
    }

    int timeout = atoi(argv[1]);

    timeval tv, tvout;
    tv.tv_sec = tvout.tv_sec = timeout;
    tv.tv_usec = tvout.tv_usec = 0;

    srand(time(NULL));

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        perror("Failed to create socket");
        return errno;
    }


    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(1505);

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);

    //https://manpages.courier-mta.org/htmlman2/select.2.html
    int status = select(sock+1, &readfds, NULL, NULL, &tv);
    if(status > 0)
    {
        tv.tv_sec = timeout;
        tv.tv_usec = 0;
        int start = time(NULL);
        int wait_for_time = 0;
        while(wait_for_time < timeout)
        {
            if(connect(sock, (sockaddr*)&addr, sizeof(addr)) == 0)
            {
                setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, 
                            (const char*)&tvout, sizeof(tvout));
                std::cout << "Connected..." << std::endl;

                std::cout << "Generating sequence..." << std::endl;
                int arr[SEQ_LEN];
                for(int i = 0; i < SEQ_LEN; ++i)
                    arr[i] = rand() % 51;

                std::cout << "Generated sequence: " << arr[0];
                for(int i = 1; i < SEQ_LEN; ++i)
                    std::cout << "," << arr[i];
                std::cout << std::endl;

                if(send(sock, arr, 4*SEQ_LEN, 0) < 0)
                {
                    perror("Failed to send array");
                    return errno;
                }

                std::cout << "Array was sent, waiting..." << std::endl << std::endl;
                if(recv(sock, arr, 4*SEQ_LEN, 0) < 0)
                {
                    perror("Failed to recieve answer");
                    return errno;
                }

                std::cout << "Sequence: ";
                for(int i = 0; i < SEQ_LEN; ++i)
                {
                    if(i)
                        std::cout << ",";

                    std::cout << arr[i];
                }

                std::cout << std::endl;
                close(sock);
                return 0;
            }
            else
                wait_for_time = time(NULL) - start;
        }

        if(wait_for_time >= timeout)
            std::cout << "TIMEOUT!" << std::endl;
    }

    close(sock);
    return 0;
}