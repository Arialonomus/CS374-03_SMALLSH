#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdint.h>
#include <err.h>
#include "parser.h"
#include "environment.h"

int main(int argc, char* argv[])
{
    /* Set default environment values */
    char* pid_str = NULL;
    asprintf(&pid_str, "%jd", (intmax_t) getpid());
    if(setenv("$", pid_str, 1) == -1)
        err(1, "setenv(): $");
    if(setenv("?", "0", 1) == -1)
        err(1, "setenv(): ?");
    if(setenv("!", "", 1) == -1)
        err(1, "setenv(): !");
    free(pid_str);
    pid_str = NULL;

    /* Prep signal handling */
    struct sigaction** dispositions = NULL;
    struct sigaction ignore;
    ignore.sa_handler = SIG_IGN;
    if (sigemptyset(&ignore.sa_mask) == -1) err(1, "sigemptyset");
    ignore.sa_restorer = NULL;

    /* Select mode based on passed-in arguments */
    // DEFAULT: Interactive Mode
    FILE* input = stdin;
    char* inputFileName = "(stdin)";
    if(argc < 2) {
        /* Set up signal handling and store previous dispositions */
        dispositions = malloc(sizeof(struct sigaction*) * NUM_IGNORED);
        for (int i = 0; i < NUM_IGNORED; ++i) {
            struct sigaction* oldact = malloc(sizeof(struct sigaction));
            if(sigaction(IGNORED[i], NULL, oldact) == -1)
                err(1, "sigaction(): store old disposition for %d", IGNORED[i]);
            dispositions[i] = oldact;
            if(sigaction(IGNORED[i], &ignore, NULL) == -1)
                err(1, "sigaction(): ignore %d", IGNORED[i]);
        }
    }
    // Non-Interactive Mode
    else if (argc == 2) {
        inputFileName = argv[1];
        input = fopen(inputFileName, "re");
        if (!input) err(1, "fopen(): %s", inputFileName);
    }
    else if (argc > 2) { errx(1, "too many arguments"); }

    /* Main Program Loop */
    char* words[MAX_WORDS] = {NULL};    // A program-level array of words representing tokenized arguments
    char* line = NULL;                  // Holds a line read from input
    size_t n = 0;                       // Holds the number of characters in line

    for (;;) {
        /* Manage background processes */
        int bg_status;
        pid_t bg_pid = waitpid(0, &bg_status, WNOHANG | WUNTRACED );
        if (bg_pid == -1 && errno != ECHILD) {
            err (1, "waitpid(): background process management");
        }
        while (bg_pid > 0) {
            if (WIFEXITED(bg_status)) {
                fprintf(stderr, "Child process %jd done. Exit status %d.\n",
                    (intmax_t) bg_pid, WEXITSTATUS(bg_status));
            }
            else if (WIFSIGNALED(bg_status)) {
                int term_sig = WTERMSIG(bg_status);
                fprintf(stderr, "Child process %jd done. Signaled %d.\n",
                    (intmax_t) bg_pid, term_sig);
            }
            else if (WIFSTOPPED(bg_status)) {
                continue_child(bg_pid);
            }
            bg_pid = waitpid(0, &bg_status, WNOHANG);
            if (bg_pid == -1 && errno != ECHILD) {
                err (1, "waitpid(): background process management");
            }
        }
        errno = 0;  // Clear potential ECHILD flag

        /* Interactive mode housekeeping */
        if (input == stdin) {
            /* Expand and print prompt string */
            char* promptStr = getenv("PS1");
            if (!promptStr) promptStr = "$";
            fprintf(stderr, "%s", promptStr);

            /* Change SIGINT disposition for line read */
            struct sigaction act;
            act.sa_handler = sigint_handler;
            if (sigfillset(&act.sa_mask) == -1) err(1, "sigfillset");
            act.sa_flags = 0;
            act.sa_restorer = NULL;
            if(sigaction(SIGINT, &act, NULL) == -1) err(1, "sigaction");
        }

        /* Read a line from input */
        ssize_t const lineLength = getline(&line, &n, input);
        if (lineLength < 0) {
            if (feof(input)) break;
            if (ferror(input) && input != stdin) err(1, "getline(): %s", inputFileName);
        }

        /* Handle read errors in interactive mode */
        if (input == stdin) {
            if (errno == EINTR) {
                clearerr(input);
                fprintf(stderr, "\n");
                continue;
            }
            if(sigaction(SIGINT, &ignore, NULL) == -1)
                err(1, "sigaction(): ignore %d", SIGINT);
        }

        /* Tokenize input line and expand parameters */
        size_t const numWords = tokenize(line, words);
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
                cmd_external(cmd, dispositions);
                break;
            default:
                break;
        }

        /* Prepare for next loop */
        for(int i = 0; i < cmd.rd_count; ++i)
            free(cmd.redirects[i]);
        free(cmd.argv);
        free(cmd.redirects);
        fflush(stdout);
    }
}
