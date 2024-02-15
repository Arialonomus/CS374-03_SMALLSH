#include "parser.h"

size_t tokenize(char const* line, char* words[]) {
    size_t word_length = 0;
    size_t index = 0;

    /* Discard leading space(s) */
    char const* char_ptr = line;
    while (*char_ptr && isspace(*char_ptr))
        ++char_ptr;

    /* Process the string to tokenize arguments */
    while (*char_ptr) {
        if (index == MAX_ARGS) break;   // End tokenizing if arg limit is exceeded
        if (*char_ptr == '#') break;    // End tokenizing to discard comments

        /* Read an argument */
        while (*char_ptr && !isspace(*char_ptr)) {
            if (*char_ptr == '\\') ++char_ptr;  // Process escaped characters

            /* Reallocate memory so the token takes exactly the amount of space it needs */
            void* tmp = realloc(words[index], sizeof **words * (word_length + 2));
            if (!tmp) err(1, "realloc");
            words[index] = tmp;
            words[index][word_length] = *char_ptr;
            ++word_length;
            words[index][word_length] = '\0';
            ++char_ptr;
        }

        /* Prepare for next argument */
        ++index;
        word_length = 0;
        while (*char_ptr && isspace(*char_ptr))
            ++char_ptr;
    }
    return index;
}

char param_scan(char const* word, char const** start, char const** end)
{
    /* Continue search from previous position if no word provided */
    static char const* prev = NULL;     // End position of the last parameter
    if (!word) word = prev;

    /* Search string for first occurence of '$' */
    char param_t = 0;   // The parameter type to be expanded
    *start = 0;
    *end = 0;
    for (char const* search_pos = word; *search_pos && !param_t; ++search_pos) {
        search_pos = strchr(search_pos, '$');
        if (!search_pos) break;
        switch (search_pos[1]) {
            /* Shell Variables */
            case '$':
            case '!':
            case '?':
                param_t = search_pos[1];
                *start = search_pos;
                *end = search_pos + 2;
                break;
            /* Custom Environment Variable */
            case '{':;
                char *end_pos = strchr(search_pos + 2, '}');
                if (end_pos) {
                    param_t = search_pos[1];
                    *start = search_pos;
                    *end = end_pos + 1;
                }
                break;
        }
    }

    /* Log ending position and return */
    prev = *end;
    return param_t;
}

char* build_str(char const* start, char const* end)
{
    static size_t base_len = 0;
    static char* base = NULL;

    /* Reset; new base string, return old one */
    if (!start) {
        char* ret = base;
        base = NULL;
        base_len = 0;
        return ret;
    }

    /* Append [start, end) to base string
     * If end is NULL, append whole start string to base string.
     * Returns a newly allocated string that the caller must free.
     */
    size_t str_len = end ? end - start : strlen(start);
    size_t newsize = sizeof *base * (base_len + str_len + 1);
    void* tmp = realloc(base, newsize);
    if (!tmp) err(1, "realloc");
    base = tmp;
    memcpy(base + base_len, start, str_len);
    base_len += str_len;
    base[base_len] = '\0';

    return base;
}

char* expand(char const* word)
{
    /* Search string for first parameter */
    char const* pos = word;
    char const* start = NULL;
    char const* end = NULL;
    char param_t = param_scan(pos, &start, &end);

    /* Build string up to first paramer occurence */
    build_str(NULL, NULL);  // Reset the base string
    build_str(pos, start);

    /* Process the string, expanding parameters as they are encountered */
    while (param_t) {
        /* Replace parameter with vale of the associated environment variable */
        if (param_t == '!') build_str(getenvstr("!"), NULL);
        else if (param_t == '$') build_str(getenvstr("$"), NULL);
        else if (param_t == '?') build_str(getenvstr("?"), NULL);
        else if (param_t == '{') {
            const size_t param_len = end - start - 3;
            char param[param_len + 1];
            strncpy(param, start + 2, param_len);
            param[param_len] = '\0';
            build_str(getenvstr(param), NULL);
        }

        /* Search for next parameter */
        pos = end;
        param_t = param_scan(pos, &start, &end);
        build_str(pos, start);
    }

    /* Append the remaining characters and return the processed string */
    return build_str(start, NULL);
}

struct redirect* check_redirect(const char* token)
{
    /* Determine if the command is a redirect */
    enum rd_t type;
    if (strcmp(token, "<") == 0) type = IN;
    else if (strcmp(token, ">") == 0)  type = OUT;
    else if (strcmp(token, ">>") == 0) type = APPEND;
    else return NULL;

    /* Allocate a struct for the redirect and return */
    struct redirect* temp_rd = malloc(sizeof(struct redirect));
    temp_rd->type = type;
    temp_rd->destination = NULL;
    return temp_rd;
}

struct command parse_command(char** tokens, const size_t n_tokens)
{
    /* Initialize command struct */
    struct command cmd = {
        EXTERNAL,
        NULL,
        malloc(sizeof(char*) * (n_tokens + 1)),
        0,
        NULL,
        0,
        false
        };
    if(!cmd.argv) err(1, "malloc");

    /* Check if last token is "&" for background process indicatior */
    int n_args = n_tokens;
    if (strcmp(tokens[n_args - 1], "&") == 0) {
        cmd.background = true;
        --n_args;
    }

    /* Iterate through tokens to parse command arguments */
    for (int i = 0; i < n_args; ++i)
    {
        /* Handle redirection */
        struct redirect* rd_ptr = check_redirect(tokens[i]);
        if (rd_ptr && cmd.type == EXTERNAL) {
            ++i;                            // Skip the redirection operator
            if (i >= n_tokens) {           // Return if operator is the last token
                free(rd_ptr);
                return cmd;
            }
            rd_ptr->destination = tokens[i];
            cmd.redirects = realloc(cmd.redirects, sizeof(struct redirect*) * (cmd.rd_count + 1));
            cmd.redirects[cmd.rd_count] = rd_ptr;
            ++cmd.rd_count;
        }
        /* Add token to argument list */
        else {
            /* Assign command name and check for built-in commands */
            if (cmd.name == NULL) {
                cmd.name = tokens[i];
                if (strcmp(cmd.name, "cd") == 0) cmd.type = CD;
                else if (strcmp(cmd.name, "exit") == 0) cmd.type = EXIT;
            }
            cmd.argv[cmd.argc] = tokens[i];
            ++cmd.argc;
        }
    }

    /* Cleanup & Exit */
    cmd.argv[cmd.argc] = NULL;
    return cmd;
}