// Author - Nícholas García

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <unistd.h>
#include <sys/wait.h>
#include <limits>

using namespace std;

//const int BUFFERSIZE = 512000;
const int BUFFERSIZE = 64000;

void emptyPipeEnd(int pipe) {
    char discard[BUFFERSIZE];
    while (read(pipe, discard, sizeof(discard)) > 0) {


    }

}

/**
 * @brief Parses input commands for later use, will ignore any extra spaces
 * @param input : string, command to parse recieved from keyboard input
 * @return vector<string> element containing all tokens, or a vector containing an empty string if input was only spaces
 */
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

void runCommand(string input_command, vector<string> parameters, string input) {
    int toChild[2];
    int toParent[2];


    if (pipe(toChild) == -1 || pipe(toParent) == -1) {
        perror("Pipe creation failed");

    }

    int parentwrite = toChild[1], childread = toChild[0];
    int childwrite = toParent[1], parentread = toParent[0];


    pid_t pid = fork();

    if (pid == 0) {
        // Child Process
        dup2(childread, 1000);
        close(childread);
        close(parentwrite);

        dup2(childwrite, 1001);
        close(parentread);
        close(childwrite);

        if (input_command.compare("miprof") == 0) { // TODO, use this to execute custom command that teacher asked for
            execl("./commands/miprof", input.c_str(), NULL);

        } else {
            execl("./commands/system_commands", input.c_str(), NULL);
        
        }

    } else {
        // Parent Process

        close(childread);
        close(childwrite);

        string input;
        /*int parameterssize = 0;
        parameterssize += parameters.size();
        input += to_string(parameterssize + 1) + "\n";*/
        
        input += input_command;
        
        for (string parameter : parameters) {
            input += " " + parameter;

        }

        input += "\n";

        write(parentwrite, input.c_str(), input.size());
        close(parentwrite);

        char buffer[BUFFERSIZE] = "";

        int status;
        waitpid(pid, &status, 0);

        read(parentread, &buffer, sizeof(buffer));
        emptyPipeEnd(parentread);
        string recieveddata = buffer;
        cout << recieveddata;

        close(parentread);

        

        if (WEXITSTATUS(status) == 137) {
            cout << "command not found";

        }

        
    }

}   

int main() {
    cout << "Prompt : ";
    for (;;) {
        string input;
        getline(cin, input);

        if (input.empty()) {
            cout << "Prompt : ";
            continue;
        
        } // Skips iteration if command is empty
        vector<string> parsed_input = parseInput(input);
        string command = parsed_input[0];
        if (input.compare("") == 0) {continue;}


        parsed_input.erase(parsed_input.begin());

        if (command.compare("exit") == 0) {
            _exit(0);
        }

        runCommand(command, parsed_input, input);     
         
        cout << "\nPrompt : ";
    }

}


