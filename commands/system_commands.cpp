#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>

using namespace std;

const int BUFFERSIZE = 64000;

void emptyPipe(int pipe) {
    char discard[BUFFERSIZE];
    while (read(pipe, discard, sizeof(discard)) > 0) {


    }

}

vector<string> parseInput(string input) {
    vector<string> parsedInput;
    
    while(input.compare("")) {
        int currentIndex = input.find(" ");

        if (currentIndex != string::npos) { /* Checks if index exists, if index == string::npos it does not exists */
            parsedInput.push_back(input.substr(0, currentIndex));
            input.erase(0, currentIndex + 1);
            for (;;) {
                char first_character = input[0];
                if (first_character == ' ') {
                    input.erase(0, 1);

                } else {break;}

            }

        }
        else {
            if (input.compare("")) { /* Checks if string is empty, if not adds remaining elements to vector */
            parsedInput.push_back(input);

            }

            break;

        }

    }

    return parsedInput;

}

void execute(vector<char*> arguments, int param) {
    int pipecomm[2];

    if (pipe(pipecomm) == -1) {
        perror("Pipe creation failed");
    }

    int piperead = pipecomm[0];
    int pipewrite = pipecomm[1];



    pid_t pid = fork();

    if (pid == 0) {
        // Child Process
        close(piperead);
        dup2(pipewrite, STDOUT_FILENO);
        close(pipewrite);

        if (param > 1) {
            execvp(arguments[0], arguments.data());
            _exit(137);

        } else {
            execlp(arguments[0], arguments[0], (char*)nullptr);

        }

    } else {
        // Parent Process
        close(pipewrite);

        int status;
        waitpid(pid, &status, 0);

        char buffer[BUFFERSIZE];
        vector<string> recieveddata;
        ssize_t n;
        read(piperead, buffer, sizeof(buffer));
        write(1001, &buffer, sizeof(buffer));

        _exit(WEXITSTATUS(status));

    }

}


int main() {
    /*int size; 
    cin >> size;*/

    char buffer[BUFFERSIZE] = "";
    read(1000, &buffer, sizeof(buffer));

    string input = buffer;

    if (input.find("\n") != string::npos) {
        input = input.substr(0, input.find("\n"));

    }

    vector<string> strarguments = parseInput(input);

    vector<char*> arguments;

    int param = 0;

    int a = 0;

    for (auto str : strarguments) {
        str = str.substr(0, str.length() / 2 + 1);
        char * copy = new char[str.size()];
        strcpy(copy, str.c_str());
        arguments.push_back(copy);
        ++param;

    }


    arguments.push_back(nullptr);
    

    execute(arguments, param);


    // for testing, delete this after done
}