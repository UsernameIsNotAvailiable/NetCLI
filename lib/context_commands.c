#include <stdio.h>
#include <inc/context_commands.h>

int get_command(const char *cmd) {

    for (int i = 0; i < COMMAND_COUNT; i++) {
        if (strcmpi(commands_list[i].command, cmd) == 0) {
            return commands_list[i].value;
        }
    }

    return COMMAND_NONE;
}