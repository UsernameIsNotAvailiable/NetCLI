#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

#include <inc/error.h>
#include <inc/log.h>
#include <inc/compile_time.h>
#include <inc/context.h>

#include <contexts/wifi/init.h>
#include <contexts/network/init.h>
#include <contexts/radio/init.h>

#include <contexts/skeleton/init.h>

#define OPT_NONE 0
#define OPT_VERSION 1
#define OPT_HELP 2
#define OPT_DEBUG 3

#define CONTEXT_NONE 0
#define CONTEXT_WIFI 1
#define CONTEXT_NETWORK 2
#define CONTEXT_RADIO 3

/*

    Skeleton context is not really
    needed. It's just for people
    new to this project to easily
    add their own context.

*/
#define CONTEXT_SKELETON 4

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

int get_context(const char *opt){
    _CONTEXT("wifi",CONTEXT_WIFI);
    _CONTEXT("network",CONTEXT_NETWORK);
    _CONTEXT("radio",CONTEXT_RADIO);
    _CONTEXT("skeleton",CONTEXT_SKELETON);

    return CONTEXT_NONE;
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
        "  network\n"
        "  wifi\n"
        "  radio\n"
        "  skeleton\n"
        ,__argv[0]
    );

}

static void opt_debug(){
    logging_enable_debug();
}

static void opt_version(){
    ncli_debug("getting build from unix timestamp...\n");
    srand(UNIX_TIMESTAMP);
    int build = rand();
    ncli_debug("done\n");

    ncli_info("NetCLI (network command line interface)\n");
    ncli_info("  build %d\n",build);
    ncli_info("  release 1.0.0\n");
    ncli_info("  unix timestamp of build: %d\n",UNIX_TIMESTAMP);
    ncli_info("  date & time of build: "__DATE__" @ "__TIME__"\n");
    ncli_info("  built with MSC version %d\n",_MSC_VER);
    ncli_info("\n  built in C <3\n");
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


int main(int argc, char *argv[]){
    SetConsoleOutputCP(CP_UTF8);

    if(argc == 1){
        ncli_info("We got no arguments, don't know what to do! Showing usage...\n");
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
    
    setup_posix_signal_handler();
    set_exception_handler();

    ncli_debug("we don't have a context yet, setting to none...\n");
    change_context("none");
    
    for(int i = 1; i < argc; i++){
            switch(get_context(argv[i])){
                case CONTEXT_WIFI:
                    ncli_debug("switching to context::wifi...\n");
                    context_wifi_entry(i,"wifi");
                    exit(0);
                    break;

                case CONTEXT_NETWORK:
                    ncli_debug("switching to context::network...\n");
                    context_network_entry(i,"network");
                    exit(0);
                    break;

                case CONTEXT_RADIO:
                    ncli_debug("switching to context::radio...\n");
                    context_radio_entry(i,"radio");
                    exit(0);
                    break;

                case CONTEXT_SKELETON:
                    ncli_debug("switching to context::skeleton...\n");
                    context_skeleton_entry(i,"skeleton");
                    exit(0);
                    break;

                case CONTEXT_NONE:
                    /*
                    
                        we have options but no context...
                        if we have options, don't raise an exception :(

                     */
                    if(have_options)
                        exit(0);

                    ncli_error("an unknown context was specified, triggering an exception...\n");
                    RaiseException(NETCLI_ERR_BAD_CONTEXT,0,0,NULL);
                    break;
        }
    }
    
}