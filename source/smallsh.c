#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "wordsplit.h"
#include "expand.h"
#include "command.h"
#include "handlers.h"

int main(int argc, char* argv[])
{
    /* Select mode based on passed-in arguments */
    // DEFAULT: Interactive Mode
    FILE* input = stdin;
    char* inputFileName = "(stdin)";
    if(argc < 2) {
        signal(SIGTSTP, SIG_IGN);
        signal(SIGINT, SIG_IGN);
    }
    // Non-Interactive Mode
    else if (argc == 2) {
        inputFileName = argv[1];
        input = fopen(inputFileName, "re");
        if (!input) err(1, "%s", inputFileName);
    }
    else if (argc > 2) { errx(1, "too many arguments"); }

    /* Main Program Loop */
    char* words[MAX_WORDS] = {NULL};    // A program-level array of words representing tokenized arguments
    char* line = NULL;                  // Holds a line read from input
    size_t n = 0;                       // Holds the number of characters in line

    for (;;) {
        /* TODO: Manage background processes */

        /* Interactive mode housekeeping */
        if (input == stdin) {
            /* Expand and print prompt string */
            char* promptStr = getenv("PS1");
            if (!promptStr) promptStr = "$";
            fprintf(stderr, "%s", expand(promptStr));

            /* Change SIGINT disposition for line read */
            struct sigaction act = {0};
            act.sa_handler = sigint_handler;
            if (sigfillset(&act.sa_mask) != 0) err(1, "sigfillset");
            act.sa_flags = 0;
            if(sigaction(SIGINT, &act, NULL) != 0) err(1, "sigaction");
        }

        /* Read a line from input */
        ssize_t const lineLength = getline(&line, &n, input);
        if (lineLength < 0) err(1, "%s", inputFileName);
        // Handle read errors in interactive mode
        if (input == stdin) {
            if (errno == EINTR) {
                clearerr(input);
                continue;
            }
            signal(SIGINT, SIG_IGN);
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
                cmd_cd(cmd.argv, cmd.argc);
                break;
            /* Built-In Command: exit */
            case EXIT:
                cmd_exit(cmd.argv, cmd.argc);
                break;
            /* External Commands */
            case EXTERNAL:
                printf("Command: External");
                break;
            default:
                break;
        }

        /* Prepare for next loop */
        //if (putchar('\n') == EOF) warn("putchar");
        free(cmd.argv);
        fflush(stdout);
    }
}
