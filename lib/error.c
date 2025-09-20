#include <Windows.h>
#include <Psapi.h>
#include <stdio.h>
#include <stdbool.h>

#include <netcli.h>
#include <inc/error.h>
#include <inc/log.h>
#include <inc/context.h>

void show_version_info_on_crash(void){
    printf("\n");
    printf("  NetCLI Release: %s, NetCLI Build: %d\n",_NETCLI_RELEASE, _NETCLI_BUILD);
    printf("  date & time of build: %s %s (unix %d)\n",__DATE__,__TIME__,UNIX_TIMESTAMP);
    printf("  compiled w/ MSC version: %d\n",_MSC_VER);
}

LONG WINAPI netcli_exception_filter(LPEXCEPTION_POINTERS excep)
{

    char exceptype[256];
    switch(excep->ExceptionRecord->ExceptionCode){

        // windows errors
        case EXCEPTION_ACCESS_VIOLATION:
            sprintf(exceptype,"EXCEPTION_ACCESS_VIOLATION");
            break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            sprintf(exceptype,"EXCEPTION_ARRAY_BOUNDS_EXCEEDED");
            break;
        case EXCEPTION_STACK_OVERFLOW:
            sprintf(exceptype,"EXCEPTION_STACK_OVERFLOW");
            break;
        case EXCEPTION_BREAKPOINT:
            sprintf(exceptype,"EXCEPTION_BREAKPOINT");
            break;
        case EXCEPTION_DATATYPE_MISALIGNMENT:
            sprintf(exceptype,"EXCEPTION_DATATYPE_MISALIGNMENT");
            break;
        case EXCEPTION_ILLEGAL_INSTRUCTION:
            sprintf(exceptype,"EXCEPTION_ILLEGAL_INSTRUCTION");
            break;

        // software errors
        case NETCLI_ERR_BAD_CONTEXT:
            sprintf(exceptype,"NETCLI_ERR_BAD_CONTEXT");
            break;

        case NETCLI_ERR_NULL_CONTEXT:
            sprintf(exceptype,"NETCLI_ERR_NULL_CONTEXT");
            break;

        //default
        default:
            sprintf(exceptype,"<error not specified in translation table>");
            break;
    }

    //get the origin
    char *origin = (char*)malloc(50);
    bool from_software = false;
    if(excep->ExceptionRecord->ExceptionFlags &EXCEPTION_SOFTWARE_ORIGINATE){
        origin = "software";
        from_software = true;
    } else {
        origin = "windows";
    }

    char arg[50];

    PVOID addr;
    bool read_violation = false;
    bool dep_violation = false;
    switch(excep->ExceptionRecord->ExceptionCode) {
        case EXCEPTION_ACCESS_VIOLATION:
            ULONG_PTR type = excep->ExceptionRecord->ExceptionInformation[0];
            addr = (PVOID)excep->ExceptionRecord->ExceptionInformation[1];
            switch(type){
                case 0:
                    //printf("\t \t read at 0x%p \n",addr);
                    read_violation = true;
                    break;

                case 1:
                    //printf("\t \t write at 0x%p \n",addr);
                    break;

                case 8:
                    dep_violation = true;
                    //printf("DEP violation at 0x%p \n",addr);
                    break;

                default:
                    //printf("unknown operation %llu at 0x%p\n",type,addr);
                    break;
            }
            break;

        default:
            //printf("\t \t no extended information available\n");
            break;
    }

    printf(_COLOR_ERROR);
    printf("\n\n********** NETCLI EXCEPTION FILTER **********\n");
    printf("  %s (0x%09X) (%s)\n",
        exceptype,
        excep->ExceptionRecord->ExceptionCode,
        from_software ? "software" : "windows"  
    );
    if(from_software){
        printf("  the software exception was triggered at 0x%p\n",excep->ExceptionRecord->ExceptionAddress);
    } else if(!dep_violation){
        printf("  the instruction at 0x%p attempted to %s 0x%p\n",
            excep->ExceptionRecord->ExceptionAddress,
            read_violation ? "read from" : "write to",
            addr
        );
    } else {
        printf(" the instruction at 0x%p triggered a DEP violation at 0x%p\n",
            excep->ExceptionRecord->ExceptionAddress,
            addr
        );
    } 
    printf("  the exception was triggered inside context::%s\n",get_current_context());

    // we don't need to continue for a software exception; debug can override this
    if(from_software & !is_debug_enabled()){
        show_version_info_on_crash();
        printf("\033[0m");
        return EXCEPTION_EXECUTE_FAULT;
    }

    LPCONTEXT c = excep->ContextRecord;

    printf("  cpu context:\n");
    printf("    CONTEXT FLAGS: %016X EFLAGS: %016X \n",c->ContextFlags,c->EFlags);
    printf("    RAX: %016llX  RBX: %016llX  RCX: %016llX \n",c->Rax,c->Rbx,c->Rcx);
    printf("    RDI: %016llX  RSI: %016llX  RBP: %016llX \n",c->Rdi,c->Rsi,c->Rbp);
    printf("    RSP: %016llX  RDX: %016llX  R10: %016llX \n",c->Rsp,c->Rdx,c->R10);
    printf("    R11: %016llX  R12: %016llX  R13: %016llX \n",c->R11,c->R12,c->R13);
    printf("    R14: %016llX  R15: %016llX  R8:  %016llX \n",c->R14,c->R15,c->R8);
    printf("    R9:  %016llX  RIP: %016llX \n",c->R9,c->Rip);

    show_version_info_on_crash();

    printf("\033[0m");
    return EXCEPTION_EXECUTE_FAULT;
}