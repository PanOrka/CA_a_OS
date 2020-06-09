#include <stdio.h>
#include <unistd.h>

int main() {
        /*
           chmod zmienil prawa dostepu do skompilowanego pliku a.out, natomiast
           my tutaj zmieniamy UID oraz GID aby miec prawa roota
           roznica miedzy effective UID (chmod) a real UID (ciagle ja)
         */
        char *args[] = {"/bin/bash", NULL};
        setuid(0);
        execvp(args[0], args);
        perror("nie mozna uruchomic Shell");

        return 0;
}
