#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sstream>

# define C_RESET	"\x1b[0m"
# define C_RED		"\x1b[38;2;255;0;0m"
# define C_GREEN	"\x1b[38;2;0;255;0m"


int main() {

    int pipeUpload[2];
    int res = pipe(pipeUpload);

    int pid;
    pid = fork();
    if (pid == 0)
    {
        // Child: Writer program
        close(pipeUpload[0]);
        dup2(pipeUpload[1], STDOUT_FILENO);
        int i = 0;
        while (i < 5) {
            sleep(1);
            std::cout << "round: " << i;
            std::cout << "aaa";
            std::cout << "bbb";
            std::cout << "ccc";
            std::cout << "ddd";
            std::cout << "eee";
            std::cout << "fff" << std::flush;
            {
                std::stringstream greenText;
                greenText << C_GREEN << "round " << i << " put into pipe" << C_RESET << "\n";
                std::cerr << greenText.str();
            }
            i++;
        }
        close(pipeUpload[1]);
        exit(0);
    }

    // Call Reader program without waiting for writing to be finished!
    std::cout << "start external program.. \n" << std::flush;

    char *execLoc = strdup("./reader");
    char **execArgv = (char **)malloc(2 * sizeof(char *));
    execArgv[0] = strdup("reader");
    execArgv[1] = NULL;

    dup2(pipeUpload[0], STDIN_FILENO); // redirected stdin, so external program will read from pipe
    res = execve(execLoc, execArgv, NULL);

    free(execLoc);
    free(execArgv[0]);
    free(execArgv);
}
