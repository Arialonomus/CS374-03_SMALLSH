#include "command.h"

struct Command parseCommand(char** tokens, size_t numTokens)
{
    /* Initialize command struct */
    struct Command cmd = {};
    cmd.argc = 0;
    cmd.argv = malloc(sizeof(char*) * (numTokens + 1));
    if(!cmd.argv) err(1, "malloc");

    /* Get command name and check for built-in commands */
    cmd.command = tokens[0];
    if (strcmp(cmd.command, "cd") == 0) cmd.cmd_t = CD;
    else if (strcmp(cmd.command, "exit") == 0) cmd.cmd_t = EXIT;
    else cmd.cmd_t = EXTERNAL;

    /* Iterate through tokens to parse command arguments */
    for (int i = 1; i < numTokens - 1; ++i)
    {
        /* Check for redirection */
        enum RD_FLAG rdType = NONE;
        if (strcmp(tokens[i], "<") == 0) rdType = RD_IN;
        else if (strcmp(tokens[i], ">") == 0)  rdType = RD_OUT;
        else if (strcmp(tokens[i], ">>") == 0) rdType = RD_APPEND;

        /* Handle redirection */
        if (rdType != NONE) ++i;            // Skip the redirection operator
        if (i >= numTokens) return cmd;     // Return if operator is the last token
        if (cmd.cmd_t == EXTERNAL) {
            switch (rdType) {
                case RD_IN:
                    cmd.inputFile = tokens[i];
                    break;
                case RD_APPEND:
                    cmd.outputFile = tokens[i];
                    cmd.append = true;
                    break;
                case RD_OUT:
                    cmd.outputFile = tokens[i];
                    cmd.append = false;
                    break;
                default:
                    break;
            }
        }
        /* Add token to argument list */
        else {
            cmd.argv[cmd.argc] = tokens[i];
            ++cmd.argc;
        }
    }

    /* Check if last token is "&" for background process indicatior */
    if (strcmp(tokens[numTokens - 1], "&")) {
        cmd.background = true;
    }
    else {
        cmd.argv[cmd.argc] = tokens[numTokens - 1];
        ++cmd.argc;
    }

    /* Cleanup & Exit */
    return cmd;
}
