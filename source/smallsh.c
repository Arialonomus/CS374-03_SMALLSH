#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

#include "wordsplit.h"
#include "command.h"

int main(int argc, char* argv[])
{
    /* Select mode based on passed-in arguments */
    // DEFAULT: Interactive Mode
    FILE* input = stdin;
    char* inputFileName = "(stdin)";
    struct sigaction* dispositions[NUM_IGNORED] = {};
    if(argc < 2) {
        for (int i = 0; i < NUM_IGNORED; ++i) {
            if(sigaction(IGNORED[i], NULL, dispositions[i]) == -1)
                err(1, "sigaction(): store old disposition for %d", IGNORED[i]);
            if(signal(IGNORED[i], SIG_IGN) == SIG_ERR) err(1, "signal(%d)", IGNORED[i]);
        }
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
            if (sigfillset(&act.sa_mask) == -1) err(1, "sigfillset");
            act.sa_flags = 0;
            if(sigaction(SIGINT, &act, NULL) == -1) err(1, "sigaction");
        }

        /* Read a line from input */
        ssize_t const lineLength = getline(&line, &n, input);
        if (lineLength < 0) err(1, "%s", inputFileName);
        // Handle read errors in interactive mode
        if (input == stdin) {
            if (errno == EINTR) {
                clearerr(input);
                if (putchar('\n') == EOF) err(1, "putchar");
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
        switch(cmd.cmd_t) {             // NOTE: All error handling is carried out within cmd_ functions
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
                pid_t child_pid;
                switch(child_pid = fork()) {
                    // Error
                    case -1:
                        warn("fork");
                    break;
                    // Child Process
                    case 0:
                        /* Reset signal dispositions */
                        for (int i = 0; i < NUM_IGNORED; ++i) {
                            if(sigaction(IGNORED[i], dispositions[i], NULL) == -1)
                                err(1, "sigaction(): restore disposition for %d", IGNORED[i]);
                        }

                    execute(cmd);
                    break;
                    // Parent Process
                    default:
                        break;
                }
                break;
            default:
                break;
        }

        /* Prepare for next loop */
        free(cmd.argv);
        fflush(stdout);
    }
}
