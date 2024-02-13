#include "command.h"

struct Command parseCommand(char** tokens, size_t numTokens)
{
    /* Initialize command struct */
    struct Command cmd = {
        EXTERNAL,
        NULL,
        malloc(sizeof(char*) * (numTokens + 1)),
        0,
        NULL,
        NULL,
        false,
        false
        };
    if(!cmd.argv) err(1, "malloc");

    /* Check if last token is "&" for background process indicatior */
    int numToParse = numTokens;
    if (strcmp(tokens[numToParse - 1], "&") == 0) {
        cmd.background = true;
        --numToParse;
    }

    /* Iterate through tokens to parse command arguments */
    for (int i = 0; i < numToParse; ++i)
    {
        /* Handle redirection */
        const enum RD_FLAG rd_t = checkRedirect(tokens[i]);
        if (rd_t != NONE && cmd.cmd_t == EXTERNAL) {
            ++i;                            // Skip the redirection operator
            if (i >= numTokens) return cmd; // Return if operator is the last token
            switch (rd_t) {
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
        /* Assign command name and check for built-in commands */
        else if (cmd.name == NULL) {
            cmd.name = tokens[i];
            if (strcmp(cmd.name, "cd") == 0) cmd.cmd_t = CD;
            else if (strcmp(cmd.name, "exit") == 0) cmd.cmd_t = EXIT;
        }
        /* Add token to argument list */
        else {
            cmd.argv[cmd.argc] = tokens[i];
            ++cmd.argc;
        }
    }

    /* Cleanup & Exit */
    return cmd;
}

enum RD_FLAG checkRedirect(const char* token)
{
    enum RD_FLAG rd_t = NONE;
    if (strcmp(token, "<") == 0) rd_t = RD_IN;
    else if (strcmp(token, ">") == 0)  rd_t = RD_OUT;
    else if (strcmp(token, ">>") == 0) rd_t = RD_APPEND;

    return rd_t;
}

void cmd_cd (char** argv, const int argc)
{
    if (argc > 1) {
        errno = E2BIG;
        warn("cd");
        return;
    }
    if (argc == 0) {
        argv[0] = getenv("HOME");
    }
    if(chdir(argv[0]) != 0) warn("chdir");
}

void cmd_exit(char** argv, const int argc)
{
    /* Validate command args */
    if (argc > 1) {
        errno = E2BIG;
        warn("exit");
        return;
    }
    if (argc == 0) {
        argv[0] = expand("$?");
    }

    /* Convert argument to status int */
    errno = 0;
    char* end = NULL;
    const long status = strtol(argv[0], &end, 10);
    if(strcmp(argv[0], end) == 0 || status > 255 || status < 0) {
        warnx("exit: Invalid argument");
        return;
    }
    if(errno != 0) {
        warn("exit");
        return;
    }
    _exit(status);
}

void execute(struct Command cmd)
{
    /* Handle redirection */
    if(cmd.inputFile) {
        int input_fd = open(cmd.inputFile, O_RDONLY );
        if (input_fd == -1) err(1, "open: %s", cmd.inputFile);
        if (dup2(input_fd, STDIN_FILENO) == -1) err(1, "source dup2()");
        close (input_fd);
    }
    if(cmd.outputFile) {
        int output_fd = open(cmd.outputFile, cmd.append ? O_WRONLY | O_APPEND : O_WRONLY, 0777);
        if (output_fd == -1) err(1, "open: %s", cmd.outputFile);
        if (dup2(output_fd, STDOUT_FILENO) == -1) err(1, "target dup2()");
        close (output_fd);
    }

    /* Execute external program */
    if(execvp(cmd.name, cmd.argv) == -1) err(1, "execvp(): %s", cmd.name);
}