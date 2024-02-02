#include "expand.h"

char param_scan(char const *word, char const **start, char const **end)
{
    static char const *prev;
    if (!word) word = prev;

    char ret = 0;
    *start = 0;
    *end = 0;
    for (char const *s = word; *s && !ret; ++s) {
        s = strchr(s, '$');
        if (!s) break;
        switch (s[1]) {
            case '$':
            case '!':
            case '?':
                ret = s[1];
                *start = s;
                *end = s + 2;
                break;
            case '{':;
                char *e = strchr(s + 2, '}');
                if (e) {
                    ret = s[1];
                    *start = s;
                    *end = e + 1;
                }
                break;
        }
    }
    prev = *end;
    return ret;
}

char* build_str(char const *start, char const *end)
{
    static size_t base_len = 0;
    static char *base = 0;

    if (!start) {
        /* Reset; new base string, return old one */
        char *ret = base;
        base = NULL;
        base_len = 0;
        return ret;
    }
    /* Append [start, end) to base string
     * If end is NULL, append whole start string to base string.
     * Returns a newly allocated string that the caller must free.
     */
    size_t n = end ? end - start : strlen(start);
    size_t newsize = sizeof *base *(base_len + n + 1);
    void *tmp = realloc(base, newsize);
    if (!tmp) err(1, "realloc");
    base = tmp;
    memcpy(base + base_len, start, n);
    base_len += n;
    base[base_len] = '\0';

    return base;
}

char* expand(char const *word)
{
    char const *pos = word;
    char const *start, *end;
    char c = param_scan(pos, &start, &end);
    build_str(NULL, NULL);
    build_str(pos, start);
    while (c) {
        if (c == '!') build_str("<BGPID>", NULL);
        else if (c == '$') build_str("<PID>", NULL);
        else if (c == '?') build_str("<STATUS>", NULL);
        else if (c == '{') {
            build_str("<Parameter: ", NULL);
            build_str(start + 2, end - 1);
            build_str(">", NULL);
        }
        pos = end;
        c = param_scan(pos, &start, &end);
        build_str(pos, start);
    }
    return build_str(start, NULL);
}