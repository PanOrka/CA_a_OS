#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

const int maxChars = 255;
const int maxStrIndex = 32;
const int maxPipes = 11;
pid_t p;


void redirectFromFile(FILE* filefd){
        dup2(fileno(filefd), 0);
        fclose(filefd);
}

void redirectToFile(FILE* filefd){
        dup2(fileno(filefd), 1);
        fclose(filefd);
}

void redirectErrToFile(FILE* filefd){
        dup2(fileno(filefd), 2);
        fclose(filefd);
}

void redirectToFiles(char** command){
        FILE* filefdIn;
        FILE* filefdOut;
        FILE* filefdErr;
        while (*command) {
                if (strcmp(*command,"<") == 0) {
                        *command = NULL;
                        filefdIn = fopen(*(command+1),"r");
                        redirectFromFile(filefdIn);
                        command+=2;
                } else if(strcmp(*command,">") == 0) {
                        *command = NULL;
                        filefdOut = fopen(*(command+1),"w");
                        redirectToFile(filefdOut);
                        command+=2;
                } else if(strcmp(*command,"2>") == 0) {
                        *command = NULL;
                        filefdErr = fopen(*(command+1),"w");
                        redirectErrToFile(filefdErr);
                        command+=2;
                }
                command++;
        }
}

int parseCommand(char input[], char* strings[]) { // parsing command without pipes
        int iterator = 0;
        for (int i=0; i<maxChars; i++) {
                if (input[i] == ' ' || input[i] == '\n') {
                        input[i] = '\0';
                } else if (input[i] == '\0') {
                        strings[iterator] = NULL;
                        return iterator;
                } else if (iterator < maxStrIndex - 1) {
                        strings[iterator] = &input[i];
                        iterator++;
                        int k = i;
                        while (k < maxChars && input[k] != ' ' && input[k] != '\n') {
                                k++;
                        }
                        i = k-1;
                }
        }
        return -1;
}

bool check(char** string, int* endOfCommand) { // check for bg (&)
        if (strcmp(*string, "&") == 0) {
                *string = NULL;
                *endOfCommand -= 1;
                printf("running in bg\n");
                return true;
        }
        return false;
}

void exec(char* strings[], bool runningInBg) { // execution without pipes
        if (strcmp(strings[0], "cd") == 0) {
                chdir(strings[1]);
        } else if (strcmp(strings[0], "exit") == 0) {
                exit(0);
        } else {
                p = fork();
                if (runningInBg) setpgid(p, p);
                if (p == 0) {
                        redirectToFiles(strings);
                        execvp(strings[0], strings);
                        perror("command error");
                        exit(1);
                } else if (!runningInBg) {
                        waitpid(p, NULL, 0);
                }
        }
}

void sigintHandler(int signum) {
        printf("\x1b[31;5;1mTERMINATED\x1b[0m\n");
// interrupts child without using kill() (?why)
        /*
        answer: A child process inherits signal settings from its parent during fork ().
        When process performs exec (), previously ignored signals remain ignored but
        installed handlers are set back to the default handler.
        */
}

int checkPipes(char input[]) { // check how many pipes
        int counter = 0;
        for (int i=0; i<maxChars; i++) {
                if (input[i] == '\n') break;
                if (counter < maxPipes-1 && input[i] == '|') { // maxPipes - 1 because
                        counter++;
                }
        }
        return counter;
}

void parsePipes(char input[], char* strings[maxPipes][maxStrIndex], int pipes) { // parser by default for many pipes
        int charCounter = 0;
        for (int i=0; i<=pipes; i++) {
                int iterator = 0;
                while (charCounter < maxChars && input[charCounter] != '|') {
                        if (input[charCounter] == ' ' || input[charCounter] == '\n') {
                                input[charCounter] = '\0';
                        } else if (input[charCounter] == '\0') {
                                strings[i][iterator] = NULL;
                                return;
                        } else if (iterator < maxStrIndex - 1) {
                                strings[i][iterator] = &input[charCounter];
                                iterator++;
                                int k = charCounter;
                                while (k < maxChars && input[k] != '|' && input[k] != ' ' && input[k] != '\n') {
                                        k++;
                                }
                                charCounter = k-1;
                        }
                        charCounter++;
                }
                if (input[charCounter] == '|') {
                        strings[i][iterator] = NULL;
                        input[charCounter] = '\0';
                        charCounter++;
                }
        }
}

void execPipes(char* strings[][maxStrIndex], int pipes) { // MULTI PIPE FUNC
        int fd[2][2];
        pipe(fd[0]);
        int i = 0;

        if (fork() == 0) {      // first fork
                dup2(fd[i][1], 1); /* this end of the pipe becomes the standard output */
                //close(fd[i][1]);
                close(fd[i][0]);

                redirectToFiles(strings[i]);

                execvp(strings[i][0], strings[i]);
                perror("command error");
                exit(1);
        }
        i++;
        for (i=1; i<pipes; i++) {
                pipe(fd[i%2]);
                if (fork() == 0) {     // ... forks
                        dup2(fd[(i-1)%2][0], 0); /* this end of the pipe becomes the standard input */
                        close(fd[(i-1)%2][1]);
                        //close(fd[(i-1)%2][0]);

                        dup2(fd[i%2][1], 1);
                        //close(fd[i%2][1]);
                        close(fd[i%2][0]);

                        redirectToFiles(strings[i]);

                        execvp(strings[i][0], strings[i]);
                        perror("command error");
                        exit(1);
                }
                close(fd[(i-1)%2][1]);
                close(fd[(i-1)%2][0]); // closing file descriptors in parent
        }

        if (fork() == 0) {      // last fork
                dup2(fd[(i-1)%2][0], 0);
                close(fd[(i-1)%2][1]);
                //close(fd[(i-1)%2][0]);

                redirectToFiles(strings[i]);

                execvp(strings[i][0], strings[i]);
                perror("command error");
                exit(1);
        }

        close(fd[(i-1)%2][1]);
        close(fd[(i-1)%2][0]); // closing file descriptors in parent
        for (int i=0; i<pipes*2; i++) { // wait for children
                wait(0);
        }
}

void executeCommand(char input[]) {
        int pipes = 0;

        pipes = checkPipes(input);

        if (input[0] != '\n' && input[0] != ' ') {
                if (pipes == 0) {
                        bool runningInBg = false;
                        char *strings[maxStrIndex];
                        int endOfCommand = parseCommand(input, strings);

                        runningInBg = check(&strings[endOfCommand-1], &endOfCommand);
                        exec(strings, runningInBg);
                } else if (pipes > 0) {
                        char *pipeStrings[maxPipes][maxStrIndex];

                        parsePipes(input, pipeStrings, pipes);
                        execPipes(pipeStrings, pipes);
                }
        }
        waitpid(-1, NULL, WNOHANG);
}

int main() {
        char cwd[PATH_MAX];
        char input[maxChars];

        signal(SIGINT, sigintHandler);
        while (true) {
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                        printf("\x1b[32;1mlsh\x1b[0m:\x1b[34;1m~%s\x1b[0m$ ", cwd);
                } else {
                        perror("getcwd() error");
                        return 1;
                }
                fgets(input, maxChars, stdin);
                executeCommand(input);
        }

        return 0;
}
