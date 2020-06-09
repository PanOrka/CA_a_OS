#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void sighandler(int signum) {
        printf("Caught signal %d, coming out...\n", signum);
}

int main() {
        for (int i=1; i<=64; i++) {
                signal(i, sighandler);
        }
        //signal(SIGINT, sighandler);
        while (true) {
                sleep(1);
        }
        return(0);
}
