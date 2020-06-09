#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

const int size = 32;

void saveInt(char* buf, int* d) { // zapis z buf do int TO DO SCANFA
        *d=0; // bo ma garbage w tym miejscu
        while (*buf != '\n') {
                *d *= 10;
                *d += *buf - '0';
                buf += 1;
        }
}

void saveCharArray(char* buf, char* str) { // zapis z buf do string TO DO SCANFA
        while (*buf != '\n') {
                *str = *buf;
                buf += 1;
                str += 1;
        }
        *str = '\0';
}

void saveBinary(char* buf, int* d) {
        int tempSize = 0;
        *d = 0;
        while (*buf != '\n') {
                buf++;
                tempSize++;
        }
        buf--;
        int multiplier = 1;

        while (tempSize != 0) {
                if (*buf == '1') {
                        *d += multiplier;
                }

                multiplier *= 2;
                tempSize--;
                buf--;
        }
}

void saveHexa(char* buf, int* d) {
        int tempSize = 0;
        *d = 0;
        while (*buf != '\n') {
                buf++;
                tempSize++;
        }
        buf--;
        int multiplier = 1;

        while (tempSize != 0) {
                if (*buf >= '0' && *buf <= '9') {
                        *d += (*buf - '0')*multiplier;
                } else {
                        *d += (*buf + 10 - 'A')*multiplier;
                }
                multiplier *= 16;

                tempSize--;
                buf--;
        }
}

void myscanf(char* type, void* var) { // scanf
        char buf[size];
        read(0, buf, size);
        if (*type == '%') {
                type++;
                if (*type == 'd') {
                        saveInt(buf, var);
                } else if (*type == 's') {
                        saveCharArray(buf, var);
                } else if (*type == 'b') {
                        saveBinary(buf, var);
                } else if (*type == 'x') {
                        saveHexa(buf, var);
                }
        }
}

///////////////////////////////// END_SCANF ////////////////////////////////////

////////////////////////////////// PRINTF //////////////////////////////////////

void intToChar(char* buf, int* d) { // TO DO PRINTFA
        char* tempPtr = buf;
        int tempInt = 0;
        int tempIntCount = *d;
        int count = 0;
        while (tempIntCount != 0) {
                tempIntCount /= 10;
                count++;
        }

        tempPtr += count + 1;
        *tempPtr = '\0';
        tempPtr--;
        while (*d != 0) {
                tempInt = *d % 10;
                *tempPtr = (tempInt + '0');
                tempPtr--;
                *d /= 10;
        }
}

void intToBinary(int* d) { // TO DO PRINTFA
        int helper = 1;
        int actual = 0;

        while (*d != 0) {
                if (*d%2 == 0) {
                        *d /= 2;
                        helper *= 10;
                } else {
                        //(*d)--;
                        *d /= 2;
                        actual += helper;
                        helper *= 10;
                }
        }

        *d = actual;
}

void intToHexa(char* str, int tempInt) { // TO DO PRINTFA
        if (tempInt == 0) {
                *str = '0';
                str++;
                *str = '\0';
                return;
        }
        int tempSize = 0;
        int tempInt2 = tempInt;
        while (tempInt2 != 0) {
                tempInt2 /= 10000;
                tempSize++;
        }
        str += tempSize;
        while (tempInt != 0) {
                str--;
                int rest = tempInt%10000;
                int restToInt = 0;
                int multiplier = 1;
                int place = 10;
                int helper = 10;
                for (int i=0; i<4; i++) { // FROM BINARY TO INT
                        helper = (rest % place)/(place/10);
                        if (helper == 1) {
                                restToInt += multiplier;
                        }
                        multiplier *= 2;
                        place *= 10;
                }
                tempInt /= 10000;
                if (restToInt >= 0 && restToInt <= 9) {
                        *str = restToInt + '0';
                } else {
                        *str = restToInt - 10 + 'A';
                }
        }
}

void myprintf(char* str, ...) {
        char** ptr = &str;
        while (*str != '\0') {
                if (*str == '%') {
                        str++;
                        if (*str == 's') {
                                ptr+=1;
                                char* test = *ptr;
                                while(*test != '\0') {
                                        write(1, test, 1);
                                        test += 1;
                                }
                        } else if (*str == 'c') {
                                ptr+=1;
                                char* test = (char*)ptr;
                                write(1, test, 1);
                        } else if (*str == 'd') {
                                ptr+=1;
                                int* test = (int*)ptr;
                                char *buff = (char*)malloc(size*sizeof(char));
                                intToChar(buff, test);
                                write(1, buff, size);
                                free(buff);
                        } else if (*str == 'b') {
                                ptr+=1;
                                int* tempInt = (int*)malloc(sizeof(int));
                                char *buff = (char*)malloc(size*sizeof(char));
                                *tempInt = *((int*)ptr);
                                intToBinary(tempInt);
                                intToChar(buff, tempInt);
                                write(1, buff, size);
                                free(buff);
                                free(tempInt);
                        } else if (*str == 'x') {
                                ptr+=1;
                                int* tempInt = (int*)malloc(sizeof(int));
                                char* buff = (char*)malloc(size*sizeof(char));
                                char* tempPtr = buff;
                                for (int i=1; i<=32; i++) { // trzeba czyscic bo sie sypie
                                        *tempPtr = '\0';
                                        tempPtr++;
                                }
                                *tempInt = *((int*)ptr);
                                intToBinary(tempInt);
                                intToHexa(buff, *tempInt);
                                write(1, buff, size);
                                free(buff);
                                free(tempInt);
                        }
                } else {
                        write(1, str, 1);
                }
                str++;
        }
}

/////////////////////////////// END_PRINTF /////////////////////////////////////

int main() {
        char buf[size];
        unsigned int d;
        myprintf("TEST INTA: \n");
        myscanf("%d", &d);
        myprintf("%d\n", d);

        myprintf("TEST Stringa: \n");
        myscanf("%s", buf);
        myprintf("%s\n", buf);

        myprintf("TEST Binary: \n");
        myscanf("%b", &d);
        myprintf("AS INTEGER: %d\n", d);
        myprintf("AS BINARY: %b\n", d);

        myprintf("TEST Hexa: \n");
        myscanf("%x", &d);
        myprintf("AS INTEGER: %d\n", d);
        myprintf("AS HEXA: %x\n", d);

        return 0;
}
