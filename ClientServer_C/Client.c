#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

const int maxChars = 255;
const int maxStrIndex = 32;

void clearBuf(char* b, int size) {
        int i;
        for (i = 0; i < size; i++)
                b[i] = '\0';
}

void sendFile(char* commands[], int client_socket, char* client_path) {
        int size;
        chdir(client_path);
        FILE* fp = fopen(commands[1], "r");
        printf("Opening file %s/%s\n", client_path, commands[1]);
        if (fp == NULL) {
                printf("File not found\n");
                send(client_socket, "0", 4, 0);
                return;
        }
        fseek(fp, 0, SEEK_END); // seek to end of file
        size = ftell(fp); // get current file pointer
        fseek(fp, 0, SEEK_SET); // seek back to beginning of file
        char buf[size];
        char sizeBuf[12];
        sprintf(sizeBuf, "%d", size);
        send(client_socket, sizeBuf, sizeof(sizeBuf), 0);

        char ch;
        for (int i=0; i<size; i++) {
                ch = fgetc(fp);
                buf[i] = ch;
        }
        send(client_socket, buf, size, 0);
        fclose(fp);
}

void recvFile(int client_socket, char* client_path) {
        int size;
        char sizeBuf[12];
        if (recv(client_socket, sizeBuf, sizeof(sizeBuf), 0) > 0) {
                chdir(client_path);
                size = atoi(sizeBuf);

                if (size == 0) {
                        printf("FILE DOESNT EXIST\n");
                        exit(1);
                } else {
                        char fileName[20];
                        printf("ENTER FILENAME:\n");
                        int valread = read(0, fileName, sizeof(fileName));
                        fileName[valread-1] = '\0';
                        printf("Saving as: %s\n", fileName);
                        int actual = 0;
                        int bytePackets = 4;
                        char buf[bytePackets];

                        FILE* fp = fopen(fileName, "ab+");
                        while (actual < size) {
                                if (recv(client_socket, buf, bytePackets, 0)) {
                                        if (actual + bytePackets > size) {
                                                fwrite(buf, size - actual, 1, fp);
                                                actual+=bytePackets;
                                        } else {
                                                fwrite(buf, bytePackets, 1, fp);
                                                actual+=bytePackets;
                                        }
                                } else {
                                        printf("receive failure on saving file\n");
                                        exit(1);
                                }
                        }
                        fclose(fp);
                }
        } else {
                printf("receive failure on saving file\n");
                exit(1);
        }
}

void parseCommand(char input[], char* strings[]) { // parsing command without pipes
        int iterator = 0;
        for (int i=0; i<maxChars; i++) {
                if (input[i] == ' ' || input[i] == '\n') {
                        input[i] = '\0';
                } else if (input[i] == '\0') {
                        strings[iterator] = NULL;
                        return;
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
}

void executeCommand(char* command[], char* client_path) {
        if (fork() == 0) {
                chdir(client_path);
                execvp(command[0], command);
                perror("failed to execute");
                exit(0);
        } else {
                wait(0);
        }
}

void changeDir(char* newDir, char* client_path) {
        chdir(client_path);
        chdir(newDir);
        getcwd(client_path, 100);
}

void clientCommand(char* command, int client_socket, char* client_path) {
        if (strncmp(command, "lpwd", 4) == 0) {
                printf("%s\n", client_path);
                send(client_socket, ":Local:\n", 8, 0);
        } else if (strncmp(command, "lcd", 3) == 0) {
                char* parsedCommand[maxStrIndex];
                parseCommand(command, parsedCommand);
                changeDir(parsedCommand[1], client_path);
                printf("Change of local directory\n");
                send(client_socket, ":Local:\n", 8, 0);
        } else if (strncmp(command, "lls", 3) == 0) {
                char* parsedCommand[maxStrIndex];
                parseCommand(command, parsedCommand);
                parsedCommand[0] = "ls\0";
                executeCommand(parsedCommand, client_path);
                send(client_socket, ":Local:\n", 8, 0);
        } else if (strncmp(command, "get", 3) == 0) {
                if (fork() == 0) {
                        send(client_socket, command, 1024, 0);
                        recvFile(client_socket, client_path);
                        printf("SUCCESS\n");
                        send(client_socket, ":Local:\n", 8, 0);
                        exit(1);
                } else {
                        wait(0);
                }
        } else if (strncmp(command, "put", 3) == 0) {
                char* parsedCommand[maxStrIndex];
                send(client_socket, command, 1024, 0);
                parseCommand(command, parsedCommand);
                sendFile(parsedCommand, client_socket, client_path);
        } else {
                send(client_socket, command, 1024, 0);
        }
}

int main() {
        int client_socket, valread;
        struct sockaddr_in address;
        char buf[1024];
        char* client_path = (char*)malloc(sizeof(char)*100);
        getcwd(client_path, 100);
        const unsigned short port = 8888;

        if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                printf("Socket creation failure");
                exit(1);
        }

        address.sin_family = AF_INET;
        address.sin_port = htons(port);

        if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) <= 0) {
                printf("Invalid server address\n");
                exit(1);
        }

        if (connect(client_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
                printf("Connection failure\n");
                exit(1);
        }

        while(1) {
                sleep(1); // wait for full errno(errno problems)
                clearBuf(buf, 1024);
                if ((valread = recv(client_socket, buf, 1024, 0)) > 0) {
                        buf[valread] = '\0';
                        printf("%s\n", buf);
                } else {
                        printf("receive failure on server respond\n");
                        exit(1);
                }
                printf("\x1b[32;1mClient: \x1b[34;1m");
                fgets(buf, 1024, stdin); // ze wzgledu na parser z lsh.c
                clientCommand(buf, client_socket, client_path);
                printf("\x1b[0m");
        }

        return 0;
}
