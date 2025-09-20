#include <Windows.h>

#include <inc/log.h>
#include <inc/context.h>

void skeleton_context_usage(void){
    /*

        Code that displays usage would go here.
        For formatting, look at other context(s),
        and go from there.

    */
}


/*

    This is where we enter from. This acts like it's own
    independent program in all said. This can be called
    anything but for consistency it's formatted as
    follows:

    ```
    int context_<context name>_entry(int argc_start,const char *context_name)
    ```

*/
int context_skeleton_entry(int argc_start,const char *context_name){
    /*
    
        It's expected that you call this function to tell
        NetCLI that you are inside of this context.

    */
    context_entry(context_name);

    /*
    
        Context init code would go here, same for argument parsing.
        Argument parsing can use the builtin or a custom one,
        depending upon how you feel.

    */


    /*
    
        Because we register this as a real context within
        netcli.c, let's tell the user what this actually 
        is and that they shouldn't be calling it as it has
        no real purpose in life.
    
    */
    ncli_info(
        "Skeleton context\n"
        "\n"
        "      .-. \n"
        "     (o.o) \n"
        "      |=| \n"
        "     __|__ \n"
        "   //.=|=.\\\\ \n"
        "  // .=|=. \\\\ \n"
        "  \\\\ .=|=. // \n"
        "   \\\\(_=_)// \n"
        "    (:| |:) \n"
        "     || || \n"
        "     () () \n"
        "     || || \n"
        "     || || \n"
        "    ==' '== \n"
        "\n"
        "This context is created to make it easy for people\n"
        "to add their own context's as it is mostly just\n"
        "comments defining how context(s) are created.\n"
        "\n"
        "Because of that, context::skeleton serves no purpose.\n"
    );
   
    /*

        NetCLI's context changer doesn't check return values, so
        you can return whatever you want. If you want to minimize
        the back tracking to the main function which will just exit,
        you could call exit(<return value>), which will render the
        return value useless.

    */
   return 0;
}