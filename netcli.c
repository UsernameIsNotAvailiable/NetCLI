
#include <Windows.h>
#include <netcli.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

#include <inc/error.h>
#include <inc/log.h>
#include <inc/compile_time.h>
#include <inc/context.h>
#include <inc/types.h>

/*

    Used to define the initilization
    functions of context's. So we
    can find them.

*/
#include <contexts/wifi/init.h>
#include <contexts/network/init.h>
#include <contexts/radio/init.h>
#include <contexts/general/init.h>
#include <contexts/dns/init.h>
#include <contexts/skeleton/init.h>

#define OPT_NONE 0
#define OPT_VERSION 1
#define OPT_HELP 2
#define OPT_DEBUG 3

static const struct netcli_context_t contexts_list[] = {
    //NAME                  ENTRY FUNCTION
    {"wifi",                context_wifi_entry},
    {"network",             context_network_entry},
    {"radio",               context_radio_entry},
    {"dns",                 context_dns_entry},
    {"general",             context_general_entry},

    // Special context(s)
    // -----------------------------
    {"no_context",          NULL},

    /*
    
        Skeleton context is not really
        needed. It's just for people
        new to this project to easily
        add their own context.
    
    */
    {"skeleton",    context_skeleton_entry},
};
#define CONTEXT_COUNT (sizeof(contexts_list)/sizeof(contexts_list[0]))

#define _OPT(optv,ret) \
    if(strcmpi(opt,optv) == 0) \
        return ret \

#define _CONTEXT _OPT

int get_option(const char *opt){
    _OPT("--version", OPT_VERSION);
    _OPT("--help",  OPT_HELP);
    _OPT("--debug", OPT_DEBUG);

    return OPT_NONE;
}

static const struct netcli_context_t *get_context_struct(const char *name) {
    for (int i = 0; i < CONTEXT_COUNT; i++) {
        if (strcmp(contexts_list[i].name, name) == 0) {
            return &contexts_list[i];
        }
    }

    return NULL;
}

void load_context(int argc_start, const struct netcli_context_t *context){
    if(context == NULL || context->entry == NULL || context->name == NULL){
        RaiseException(NETCLI_ERR_NULL_CONTEXT,0,0,NULL);
    }
    ncli_debug("switching to context::%s...\n",context->name);

    context->entry(argc_start,context->name);
    
    exit(0);
}

static void netcli_usage(void)
{
    ncli_info(
        "usage: %s [OPTION(S)] CONTEXT { command | help }\n"
        "\n"
        "OPTIONS\n"
        "  --debug          Show debug logs.\n"
        "  --help           Show this help.\n"
        "  --version        Shows version information.\n"
        "\n"
        "CONTEXTS\n"
        "  general\n"
        "  network\n"
        "  wifi\n"
        "  radio\n"
        "  dns\n"
        "  skeleton\n"
        ,__argv[0]
    );

    exit(0);
}

static void opt_debug(){
    logging_enable_debug();
}

static void opt_version(){
    ncli_info("NetCLI (network command line interface)\n");
    ncli_info("  release %s\n",_NETCLI_RELEASE);
    ncli_info("  build %d\n",_NETCLI_BUILD);
    ncli_info("  hotfix %d, patch %d\n",_NETCLI_HOTFIX,_NETCLI_PATCH);
    ncli_info("  unix timestamp of build: %d\n",UNIX_TIMESTAMP);
    ncli_info("  date & time of build: "__DATE__" @ "__TIME__"\n");
    ncli_info("  built with MSC version %d\n",_MSC_VER);
    ncli_info("\n");
    ncli_info("  built with and only C <3\n");
    ncli_info("  contribute at https://github.com/UsernameIsNotAvailiable/NetCLI\n");
}

static inline bool is_option(const char *arg){
    return (strncmp(arg,"--",2) == 0);
}

void posix_signal_handler(int sig){
    switch(sig){
        case SIGINT:
            ncli_info("got SIGINT... terminating program\n");
            exit(0);
            break;

        case SIGSEGV:
            ncli_error("got SIGSEGV... terminating program\n");
            exit(0);
            break;

        case SIGABRT:
            ncli_info("got SIGABRT... terminating program\n");
            exit(0);
            break;

        default:
            ncli_error("unable to handle signal: %d... ignoring\n",sig);
            break;
    }
}

#define _set_sig_handler(sig) \
    signal(sig, posix_signal_handler); \
    ncli_debug("set handler for %s...\n",#sig) 

void setup_posix_signal_handler(void){
    _set_sig_handler(SIGSEGV);
    _set_sig_handler(SIGINT);
    _set_sig_handler(SIGABRT);

}

void set_exception_handler(void){
    SetUnhandledExceptionFilter(netcli_exception_filter);
    ncli_debug("set exception handler to netcli_exception_filter()\n");
}

void show_context_list_dbg(void){
    ncli_debug("contexts inside of contexts_list[]:\n");
    for(int i = 0; i < CONTEXT_COUNT; i++){
        ncli_debug("\tname=\"%s\", entry=0x%p\n", contexts_list[i].name, contexts_list[i].entry);
    }
}


int main(int argc, char *argv[]){
    /*
    
        This is for the error handler.
        So it has an origin context.
    
    */
   change_context("no_context");

   WSADATA wsa;
   WSAStartup(MAKEWORD(2,2), &wsa);

    SetConsoleOutputCP(CP_UTF8);

    if(argc == 1){
        ncli_info("We have no arguments, don't know what to do! Showing usage...\n");
        printf("\n");
        netcli_usage();
    }
    BOOL have_options = false;

    // we should parse options first...
    for(int i = 1; i < argc; i++){
        if(is_option(argv[i])) {
            have_options = true;
            switch(get_option(argv[i])){
                case OPT_DEBUG:
                    opt_debug();
                    break;

                case OPT_VERSION:
                    opt_version();
                    break;

                case OPT_HELP:
                    netcli_usage();
                    break;

                case OPT_NONE:

                default:
                    ncli_error("invalid option!");
                    break;
            }
        }
    }
    
    ncli_debug("do setup...\n");
    set_exception_handler();
    setup_posix_signal_handler();
    show_context_list_dbg();

    for(int i = 1; i < argc; i++){
        if(get_context_struct(argv[i]) == NULL){
            continue;
        }
        load_context(i,get_context_struct(argv[i]));
    }

    if(!have_options)
        RaiseException(NETCLI_ERR_BAD_CONTEXT,0,0,NULL);
    

    ncli_debug("netcli exiting...\n");
    return 0;
}