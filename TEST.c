#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]){
    printf("hi\n");
    
    char *buf = malloc(3);
    strcpy(buf,argv[1]);

    printf("%s\n",buf);


    free(buf);

    return 0;
}