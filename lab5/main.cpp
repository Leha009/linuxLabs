#include <stdlib.h>
#include <signal.h>
#include <iostream>

void SignalHandler(int iSignal)
{
    if(iSignal == SIGFPE)
    {
        perror("Division by zero!");
        exit(1);
    }
    else if(iSignal == SIGSEGV)
    {
        perror("Segmentation violation");
        exit(2);
    }
}

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        std::cout << "You forgot parameter: error code!" << std::endl;
        return 0;
    }

    int iSignal = atoi(argv[1]);
    if(iSignal < 1 || iSignal > 32)
    {
        std::cout << "Error code must be more than 0 and less than 33!" << std::endl;
        return 0;
    }

    signal(iSignal, SignalHandler);

    if(iSignal == SIGFPE)   //8
        int iDumbDivision = 1/0;
    else if(iSignal == SIGSEGV) //11
    {
        char* str = NULL;
        std::cout << "Getting data from NULL =)" << std::endl << *str;
    }
    return 0;
}