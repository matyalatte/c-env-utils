#include <stdio.h>
#include "env_utils.h"

enum {
    CMD_UNK,
    CMD_EXE_PATH,
};

int main(int argc, char **argv) {
    int cmd = CMD_EXE_PATH;
    if (argc == 1)
        return 0;

    if (argv[1][0] == 'e')
        cmd = CMD_EXE_PATH;
    else
        cmd = CMD_UNK;

    if (cmd == CMD_UNK) {
        printf("Unknown command detected. %s\n", argv[1]);
        return 1;
    } else if (cmd == CMD_EXE_PATH) {
        printf("%s\n", envuGetExecutablePath());
    }
    return 0;
}
