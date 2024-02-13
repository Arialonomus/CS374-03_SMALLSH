#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "wordsplit.h"
#include "expand.h"
#include "command.h"

int main(int argc, char* argv[])
{
    /* Select mode based on passed-in arguments */
    // DEFAULT: Interactive Mode
    FILE* input = stdin;
    char* inputFileName = "(stdin)";
    // Non-Interactive Mode
    if (argc == 2)
    {
        inputFileName = argv[1];
        input = fopen(inputFileName, "re");
        if (!input) err(1, "%s", inputFileName);
    }
    else if (argc > 2) { errx(1, "too many arguments"); }

    /* Main Program Loop */
    char* words[MAX_WORDS] = {NULL};    // A program-level array of words representing tokenized arguments
    char* line = NULL;                  // Holds a line read from input
    size_t n = 0;                       // Holds the number of characters in line

    for (;;)
    {
        /* TODO: Manage background processes */

        /* Expand and print interactive mode prompt */
        if (input == stdin) {
            char* promptStr = getenv("PS1");
            if (!promptStr) promptStr = "$";
            fprintf(stderr, "%s", expand(promptStr));
        }

        /* Read a line from input */
        ssize_t const lineLength = getline(&line, &n, input);
        if (lineLength < 0) {
            warn("%s", inputFileName);
            continue;
        }

        /* Tokenize input line and expand parameters */
        size_t const numWords = wordsplit(line, words);
        if (numWords < 1) continue;     // Skip processing for empty commands
        for (size_t i = 0; i < numWords; ++i) {
            char* expandedWord = expand(words[i]);
            free(words[i]);
            words[i] = expandedWord;
        }

        /* Parse and execute command */
        struct Command const cmd = parseCommand(words, numWords);
        if (cmd.name == NULL) continue; // Abort processing for malformed command
        switch(cmd.cmd_t) {
            /* Built-In Command: cd */
            case CD:
                if (cmd.argc > 1) {
                    errno = E2BIG;
                    warn("cd");
                    break;
                }
                if (cmd.argc == 0) {
                    cmd.argv[0] = expand(getenv("HOME"));
                }
                if(chdir(cmd.argv[0]) != 0) warn("chdir");
                break;
            /* Built-In Command: exit */
            case EXIT:
                printf("Command: EXIT");
                break;
            /* External Commands */
            case EXTERNAL:
                printf("Command: External");
                break;
            default:
                break;
        }

        /* Prepare for next loop */
        if (putchar('\n') == EOF) warn("putchar");
        free(cmd.argv);
        fflush(stdout);
    }
}
