#include <Windows.h>
#include <stdio.h>
#include <inc/utils.h>

void bar(int l){
    for(int i = 0; i < l; i++){
        printf(BOX_HLINE);
    }
    printf("\n");
}