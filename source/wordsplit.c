#include "wordsplit.h"

size_t wordsplit(char const *line) {
    size_t wlen = 0;
    size_t wind = 0;

    char const *c = line;
    for (;*c && isspace(*c); ++c); /* discard leading space */

    for (; *c;) {
        if (wind == MAX_WORDS) break;
        /* read a word */
        if (*c == '#') break;
        for (;*c && !isspace(*c); ++c) {
            if (*c == '\\') ++c;
            void *tmp = realloc(words[wind], sizeof **words * (wlen + 2));
            if (!tmp) err(1, "realloc");
            words[wind] = tmp;
            words[wind][wlen++] = *c;
            words[wind][wlen] = '\0';
        }
        ++wind;
        wlen = 0;
        for (;*c && isspace(*c); ++c);
    }
    return wind;
}