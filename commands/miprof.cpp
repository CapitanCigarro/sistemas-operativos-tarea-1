#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <sys/resource.h>
#include <sys/time.h>
#include <fstream>

using namespace std;

const int BUFFERSIZE = 64000;

vector<vector<string>> parseInput(string input) {
    vector<string> parsedInput, commandsInput;
    input = input.substr(7);
    int aux;
    int i = 1;

    if (input.substr(0, 8).compare("ejecsave") == 0) {
        parsedInput.push_back("ejecsave");
        input = input.substr(9);
        parsedInput.push_back(input.substr(0, input.find(" ") + 1));

    } else if (input.substr(0, 8).compare("ejecutar") == 0) {
        parsedInput.push_back("ejecutar");
        input = input.substr(9);
        parsedInput.push_back(input.substr(0, input.find(" ") + 1));

    } else if (input.substr(0, 5).compare("ejec") == 0) {
        input = input.substr(5);
        parsedInput.push_back("ejec");

    }

    while(input.compare("")) {
        int currentIndex = input.find(" ");
        if (currentIndex != string::npos) { /* Checks if index exists, if index == string::npos it does not exists */
            if (i > aux) {
                commandsInput.push_back(input.substr(0, currentIndex));
                input.erase(0, currentIndex + 1);
                for (;;) {
                    char first_character = input[0];
                    if (first_character == ' ') {
                        input.erase(0, 1);

                    } else {break;}

                }
            
            } else {


            }

            ++i;

        }
        else {
            if (input.compare("")) { /* Checks if string is empty, if not adds remaining elements to vector */
            commandsInput.push_back(input);

            }

            break;

        }

    }

    return {parsedInput, commandsInput};

}

void execute(vector<char*> arguments, int param, string input, vector<string> miprofargs) {
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
        dup2(pipewrite, 1005);
        close(pipewrite);

        // Child Process

        int auxpipe[2];
        pipe(auxpipe);

        int readp = auxpipe[0];
        int writep = auxpipe[1];

        struct timeval start, end;
        struct rusage usage;

        gettimeofday(&start, nullptr);

        pid_t pd = fork();

        if (pd == 0) {
            // Grandchild Process
            close(readp);
            dup2(writep, STDOUT_FILENO);
            close(writep);

            if (param > 1) {
                execvp(arguments[0], arguments.data());
                _exit(137);

            } else {
                execlp(arguments[0], arguments[0], (char*)nullptr);

            }

        } else {
            // Child Process
            close(writep);

            double elapsed = 0;
            double interval = 0.05;
            bool done = false;
            int status, status2;
            string buffermessage, message = "";
            char buffer[64000];

            if (miprofargs.size() > 1) {

                if (miprofargs[1].compare("ejecutar") == 0) {
                    double timelimit = stod(miprofargs[2]);
                    while (elapsed < timelimit) {
                        pid_t result = waitpid(pd, &status2, WNOHANG);
                        if (result == pd) {
                            done = true;
                            break;

                        }
                        usleep((useconds_t)(interval * 1e6));
                        elapsed += interval;

                    }

                    if (done == false) {
                        kill(pd, SIGKILL);
                        wait4(pd, &status, 0, &usage);


                    }

                }
                
            } 

            read(readp, buffer, sizeof(buffer));
            buffermessage = buffer;

            wait4(pd, &status, 0, &usage);

            gettimeofday(&end, nullptr);

            string realtime = to_string(((end.tv_sec - start.tv_sec) + (end.tv_sec - start.tv_sec) / 1e6));
            string usertime = to_string(usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6);
            string systime = to_string(usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6);
            string maxrss = to_string(usage.ru_maxrss);

            message = "Input : " + input + "\n" + "Real Time : " + realtime + "\n" + "User Time :" + usertime + "\n"; 
            message += "System Time : " + systime + "\n" + "Maximum Resident Set : " + maxrss + " KB\n" + "Console Message :\n" + buffermessage;

            if (miprofargs.size() > 1) {

                if (miprofargs[1].compare("ejecsave") == 0) {
                    ofstream textfile(miprofargs[2], ios::app);
                    textfile << message;
                    textfile.close();

                }

            }

            char * buffer2 = new char[message.size()];
            strcpy(buffer2, message.c_str());
            cout << buffer2;

        }

    } else {
        // Parent Process
        close(pipewrite);

        piperead = 1008;

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

    vector<vector<string>> parsed = parseInput(input);

    vector<string> strarguments = parsed[1], commandargs, miprofargs = parsed[0];

    vector<char*> arguments;

    int param = 0;

    for (auto str : commandargs) {
        char * copy = new char[str.size()];
        strcpy(copy, str.c_str());
        arguments.push_back(copy);
        ++param;

    }


    arguments.push_back(nullptr);
    

    execute(arguments, param, input, miprofargs);


    // for testing, delete this after done
}