#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <string.h>
#include "wordsplit.h"
#include "expand.h"

int main(int argc, char *argv[])
{
    FILE* input = stdin;
    char* input_fn = "(stdin)";
    if (argc == 2) {
        input_fn = argv[1];
        input = fopen(input_fn, "re");
        if (!input) err(1, "%s", input_fn);
    } else if (argc > 2) {
        errx(1, "too many arguments");
    }

    char* line = NULL;
    size_t n = 0;
    for (;;) {
//prompt:;
        /* TODO: Manage background processes */

        /* TODO: prompt */
        if (input == stdin) {

        }
        ssize_t line_len = getline(&line, &n, input);
        if (line_len < 0) err(1, "%s", input_fn);

        size_t nwords = wordsplit(line);
        for (size_t i = 0; i < nwords; ++i) {
            fprintf(stderr, "Word %zu: %s\n", i, words[i]);
            char *exp_word = expand(words[i]);
            free(words[i]);
            words[i] = exp_word;
            fprintf(stderr, "Expanded Word %zu: %s\n", i, words[i]);
        }
    }
}
