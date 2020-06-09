#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

static int x1, y1, x2, y2;
static int x3, y3;
pthread_mutex_t counterMutex;
int rowCounter = 0;

typedef struct {
        bool **m1;
        bool **m2;
        bool **m3;
} Thread_struct;

void createMatrix(bool **m1, bool **m2, bool **m3) {
        int zarodek;
        time_t tt;
        zarodek = time(&tt);
        srand(zarodek);
        bool m2Temp[x2][y2];

        bool **tempPP = m1;
        for (int i=0; i<x1; i++) {
                bool *tempP = *tempPP;
                for (int k=0; k<y1; k++) {
                        *tempP = (bool)(rand()%2);
                        printf("%d ", *tempP);
                        tempP++;
                }
                tempPP++;
                printf(" 1 \n");
        }

        printf("\n");

        for (int i=0; i<x2; i++) {
                for (int k=0; k<y2; k++) {
                        m2Temp[i][k] = (bool)(rand()%2);
                        printf("%d ", m2Temp[i][k]);
                }
                printf(" 2 \n");
        }

        printf("\n");

        tempPP = m2;
        for (int i=0; i<y2; i++) { // TRANSPOZYCJA
                bool *tempP = *tempPP;
                for (int k=0; k<x2; k++) {
                        *tempP = m2Temp[k][i];
                        printf("%d ", *tempP);
                        tempP++;
                }
                tempPP++;
                printf(" T \n");
        }

        printf("\n");

        tempPP = m3;
        for (int i=0; i<x3; i++) {
                bool *tempP = *tempPP;
                for (int k=0; k<y3; k++) {
                        *tempP = false;
                        printf("%d ", *tempP);
                        tempP++;
                }
                tempPP++;
                printf(" 3 \n");
        }

        printf("\n");
}

void *matrixCalc(void *arg) {
        int row = 0;
        Thread_struct *t = (Thread_struct *) arg;

        while (true) {
                pthread_mutex_lock(&counterMutex);
                row = rowCounter;
                rowCounter++;
                pthread_mutex_unlock(&counterMutex);

                if (row >= x1) {
                        return NULL;
                }

                bool *m3 = *((t->m3) + row);
                bool *m1 = *((t->m1) + row);
                bool **m2 = (t->m2);
                for (int i=0; i<y2; i++) {
                        bool *tempP1 = m1;
                        bool *tempP2 = *m2;
                        for (int k=0; k<x2; k++) { // x2=y1
                                if (*tempP1 && *tempP2) {
                                        *m3 = true;
                                        continue;
                                }
                                tempP1++;
                                tempP2++;
                        }
                        m3++;
                        m2++;
                }
        }
        return NULL;
}

int main () {
        int nthreads = 0;
        printf("x1: ");
        scanf("%d", &x1);
        printf("y1, x2: ");
        scanf("%d", &y1);
        x2 = y1;
        printf("y2: ");
        scanf("%d", &y2);
        x3 = x1;
        y3 = y2;

        bool **m1 = (bool **)malloc(sizeof(bool *) * x1);
        for (int i=0; i<x1; i++) {
                m1[i] = (bool*)malloc(sizeof(bool) * y1);
        }

        bool **m2 = (bool **)malloc(sizeof(bool *) * y2);
        for (int i=0; i<y2; i++) {
                m2[i] = (bool*)malloc(sizeof(bool) * x2);
        }

        bool **m3 = (bool **)malloc(sizeof(bool *) * x3);
        for (int i=0; i<x3; i++) {
                m3[i] = (bool*)malloc(sizeof(bool) * y3);
        }

        createMatrix(m1, m2, m3);

        Thread_struct *t = (Thread_struct *)malloc(sizeof(Thread_struct));
        t->m1 = m1;
        t->m2 = m2;
        t->m3 = m3;

        // Tutaj zaczyna sie zabawa
        printf("Insert amount of threads: ");
        scanf("%d", &nthreads);

        pthread_t rowCalculator[nthreads];
        for (int i=0; i<nthreads; i++) {
                if (pthread_create(&rowCalculator[i], NULL, matrixCalc, t)) {
                        printf("error in thread creation");
                        abort(); // fatal error exit
                }
        }

        for (int i=0; i<nthreads; i++) {
                if (pthread_join(rowCalculator[i], NULL)) {
                        printf("error in thread ending");
                        abort();
                }
        }

        bool **tempPP = m3;
        for (int i=0; i<x3; i++) {
                bool *tempP = *tempPP;
                for (int k=0; k<y3; k++) {
                        printf("%d ", *tempP);
                        tempP++;
                }
                tempPP++;
                printf(" = \n");
        }

        printf("\n");
}
