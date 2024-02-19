#include <iostream>
#include <unistd.h>
#include <strings.h>

int main()
{
    std::cout << "printer start!" << std::endl;
    char buffer[100];

    // read from stdin
    int j = 0;
    int res;
    while (j < 15) {
        bzero(buffer, 100);
        res = read(0, buffer, 5);  // NOTE: read is waiting if there is nothing to read!
        std::cout << "buffer: " << buffer << std::endl;
        j++;
    }
    sleep(6); // dummy to make sure writing finished
    std::cout << "printer end!" << std::endl;
}
