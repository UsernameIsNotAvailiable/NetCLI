#include <Windows.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <inc/arg.h>
#include <inc/log.h>

#define argv __argv
#define argc __argc

bool does_arg_exist(const char *arg,int *index){
    bool found = false;

    for(int i = 1; i < __argc; i++){
        if(strcmp(argv[i],arg) == 0){
            if(index != NULL){
                *index = i;
            }
            found = true;
            break;
        }
    }
    return found;
}

bool does_arg_exist_i(const char *arg,int *index){
    bool found = false;

    for(int i = 1; i < __argc; i++){
        if(strcmpi(argv[i],arg) == 0){
            if(index != NULL){
                *index = i;
            }
            found = true;
            break;
        }
    }
    return found;
}