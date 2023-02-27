#include <stdio.h>

int main(int argc, char** argv){
    if(argc < 3){
        printf("Syntax: %s <disk image> <filename>\n", *argv);
        return -1;
    }

    return 0;
}