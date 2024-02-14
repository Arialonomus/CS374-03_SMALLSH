#include "parser.h"

size_t tokenize(char const* line, char** words) {
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
        if (c == '!') build_str(getenvstr("!"), NULL);
        else if (c == '$') build_str(getenvstr("$"), NULL);
        else if (c == '?') build_str(getenvstr("?"), NULL);
        else if (c == '{') {
            const size_t param_len = end - start - 3;
            char param[param_len + 1];
            strncpy(param, start + 2, param_len);
            param[param_len] = '\0';
            build_str(getenvstr(param), NULL);
        }
        pos = end;
        c = param_scan(pos, &start, &end);
        build_str(pos, start);
    }
    return build_str(start, NULL);
}

struct Redirect* checkRedirect(const char* token)
{
    /* Determine if the command is a redirect */
    enum redirect_t type;
    if (strcmp(token, "<") == 0) type = IN;
    else if (strcmp(token, ">") == 0)  type = OUT;
    else if (strcmp(token, ">>") == 0) type = APPEND;
    else return NULL;

    /* Allocate a struct for the redirect and return */
    struct Redirect* temp_rd = malloc(sizeof(struct Redirect*));
    temp_rd->type = type;
    temp_rd->destination = NULL;
    return temp_rd;
}

struct Command parseCommand(char** tokens, size_t numTokens)
{
    /* Initialize command struct */
    struct Command cmd = {
        EXTERNAL,
        NULL,
        malloc(sizeof(char*) * (numTokens + 1)),
        0,
        NULL,
        0,
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
        struct Redirect* rd_ptr = checkRedirect(tokens[i]);
        if (rd_ptr && cmd.cmd_t == EXTERNAL) {
            ++i;                            // Skip the redirection operator
            if (i >= numTokens) {           // Return if operator is the last token
                free(rd_ptr);
                return cmd;
            }
            rd_ptr->destination = tokens[i];
            cmd.redirects = realloc(cmd.redirects, sizeof(struct Redirect*) * (cmd.rd_count + 1));
            cmd.redirects[cmd.rd_count] = rd_ptr;
            ++cmd.rd_count;
        }
        /* Add token to argument list */
        else {
            /* Assign command name and check for built-in commands */
            if (cmd.name == NULL) {
                cmd.name = tokens[i];
                if (strcmp(cmd.name, "cd") == 0) cmd.cmd_t = CD;
                else if (strcmp(cmd.name, "exit") == 0) cmd.cmd_t = EXIT;
            }
            cmd.argv[cmd.argc] = tokens[i];
            ++cmd.argc;
        }
    }

    /* Cleanup & Exit */
    cmd.argv[cmd.argc] = NULL;
    return cmd;
}