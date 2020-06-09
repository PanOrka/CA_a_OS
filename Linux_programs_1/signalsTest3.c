#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int i = 0;

void sighandler(int signum) {
        printf("Caught signal %d, #%d...\n", signum, i);
        i++;
}

int main() {
        signal(SIGINT, sighandler);
        while (true) {
                sleep(1);
        }
        return(0);
}
