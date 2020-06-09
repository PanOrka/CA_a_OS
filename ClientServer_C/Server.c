#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <stdbool.h>
#include <sys/wait.h>

const int maxChars = 255;
const int maxStrIndex = 32;

void clearBuf(char* b, int size) {
        int i;
        for (i = 0; i < size; i++)
                b[i] = '\0';
}

void recvFile(char *commands[], int client_socket, char* client_path) {
        int size;
        char sizeBuf[12];
        if (recv(client_socket, sizeBuf, sizeof(sizeBuf), 0) > 0) {
                chdir(client_path);
                size = atoi(sizeBuf);

                if (size == 0) {
                        send(client_socket, "Fail", 4, 0);
                        exit(1);
                } else {
                        printf("Saving as: %s\n", commands[2]);
                        int actual = 0;
                        int bytePackets = 4;
                        char buf[bytePackets];

                        FILE* fp = fopen(commands[2], "ab+");
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
                                        send(client_socket, "Fail", 4, 0);
                                        exit(1);
                                }
                        }
                        fclose(fp);
                }
        } else {
                printf("receive failure on saving file\n");
                send(client_socket, "Fail", 4, 0);
                exit(1);
        }
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

void executeCommand(char* command[], char* client_path, int client_socket) {
        if (fork() == 0) {
                dup2(client_socket, STDOUT_FILENO); /* duplicate socket on stdout */
                dup2(client_socket, STDERR_FILENO); // sometimes doesnt work, client has to wait a sec
                close(client_socket);
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
        char* parsedCommand[maxStrIndex];
        parseCommand(command, parsedCommand);

        if (strcmp(parsedCommand[0], "pwd") == 0) {
                send(client_socket, client_path, strlen(client_path), 0);
        } else if (strcmp(parsedCommand[0], "cd") == 0) {
                changeDir(parsedCommand[1], client_path);
                send(client_socket, "Change of directory", 19, 0);
        } else if (strcmp(parsedCommand[0], "ls") == 0) {
                executeCommand(parsedCommand, client_path, client_socket);
        } else if (strcmp(parsedCommand[0], "get") == 0) {
                sendFile(parsedCommand, client_socket, client_path);
        } else if (strcmp(parsedCommand[0], "put") == 0) {
                if (fork() == 0) {
                        recvFile(parsedCommand, client_socket, client_path);
                        send(client_socket, "Success", 7, 0);
                        exit(1);
                } else {
                        wait(0);
                }
        } else if (strcmp(parsedCommand[0], ":Local:") == 0) {
                send(client_socket, ":Local:", 7, 0);
        } else {
                send(client_socket, "Command failure", 15, 0);
        }
}

int main() {
        int opt = 1;
        int master_socket, addrlen, new_socket, client_socket[30],
            max_clients = 30, activity, sd;
        bool client_accepted[30];
        int max_sd;
        struct sockaddr_in address;
        const unsigned short int port = 8888;
        const int max_queue = 4;
        const char* password = "1234\n";
        char* client_path[30];

        for (int i=0; i<30; i++) { // setting client_paths to actual server path
                client_path[i] = (char *)malloc(sizeof(char)*100);
                if (getcwd(client_path[i], 100) != NULL) {
                } else {
                        perror("getcwd() error");
                        exit(1);
                }
        }

        char buf[1024];
        fd_set read_fds;
        char* msg = "Hello. Insert password:";

        for (int i=0; i<max_clients; i++) {
                client_socket[i] = 0;
        }

        if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                perror("Master socket failure");
                exit(1);
        }

        if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) { // REUSE but not needed
                perror("problems with setsockopt");                                              // multiple connections
                exit(1);
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        address.sin_port = htons(port); // setting to port 8888

        // binding master_socket to sockaddr_in address
        if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
                perror("bind failed");
                exit(1);
        }

        printf("Server is listening to port %d \n", port);

        // max pending connections to master_socket
        if (listen(master_socket, max_queue) < 0) {
                perror("listen failure");
                exit(1);
        }

        addrlen = sizeof(address);
        printf("Waiting for connections...\n");

        while (1) {
                // clears socket set
                FD_ZERO(&read_fds);

                // add master_socket to set
                FD_SET(master_socket, &read_fds);
                // setting highest file descriptor for select
                max_sd = master_socket;

                // adding child sockets to set
                for (int i=0; i<max_clients; i++) {
                        // socket descriptor
                        sd = client_socket[i];

                        // if valid socket descriptor then add to read list
                        if (sd >= 0) {
                                FD_SET(sd, &read_fds);
                        }

                        if (sd > max_sd) {
                                max_sd = sd;
                        }
                }

                // setting timeout as NULL, select blocks till activity occurs
repeat_select:
                activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);

                if ((activity < 0) && errno != EINTR) {
                        printf("select failure\n");
                } else if (errno == EINTR) {
                        goto repeat_select;
                }

                if (FD_ISSET(master_socket, &read_fds)) {
                        if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                                perror("accept failure");
                                exit(1);
                        }

                        if (!FD_ISSET(new_socket, &read_fds)) {
                                printf("New connection, socket fd: %d, ip is: %s, port: %d\n",
                                       new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                                // send greeting msg
                                if (send(new_socket, msg, strlen(msg), 0) != strlen(msg)) {
                                        perror("send failure");
                                }

                                printf("Welcome message sent succesfully\n");

                                // add new socket to array of sockets
                                for (int i=0; i<max_clients; i++) {
                                        // if is empty then add
                                        if (client_socket[i] == 0) {
                                                client_socket[i] = new_socket;
                                                client_accepted[i] = false;
                                                printf("Adding to list of sockets as %d\n", i);
                                                break;
                                        }
                                }
                        }
                }

                // IO on sockets
                for (int i=0; i<max_clients; i++) {
                        clearBuf(buf, 1024);
                        sd = client_socket[i];
                        // CHECKING Wheter is in set
                        if (FD_ISSET(sd, &read_fds)) {
                                if (recv(sd, buf, 1024, 0) <= 0) {
                                        // somebody disconected
                                        getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                                        printf("Host disconnected, ip: %s, port %d \n",
                                               inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                                        // closing socket
                                        close(sd);
                                        client_socket[i] = 0;
                                        client_accepted[i] = false;
                                } else {
                                        if (client_accepted[i] == true) {
                                                printf("Incoming: %s from sd: %d\n", buf, client_socket[i]);
                                                clientCommand(buf, client_socket[i], client_path[i]);
                                        } else {
                                                if (strcmp(password, buf) == 0) {
                                                        client_accepted[i] = true;
                                                        send(sd, "Accepted", 8, 0);
                                                } else {
                                                        send(sd, msg, strlen(msg), 0);
                                                }
                                        }
                                }
                        }
                }
        }

        return 0;
}
