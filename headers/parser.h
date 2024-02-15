/* Program: CS 374 Assignment 03 - SMALLSH
 *
 * This file contains constants and functions related to parsing a command
 * from lines of input, including tokenizing, parameter expansion, and
 * generating command structures from tokenized input
 *
 * Starter code provided by OSU Instructor for functions: tokenize(), param_scan()
 * build_str(), and expand().
 */

#ifndef SMALLSH_PARSER_H
#define SMALLSH_PARSER_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

// Maximum level of tokenized arguments allowed
#ifndef MAX_ARGS
#define MAX_ARGS 512
#endif

#include <signal.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <unistd.h>
#include "environment.h"
#include "command.h"

/* Splits a string into tokens delimited by whitespace. Recognizes
 * comments as '#' at the beginning of a word, and backslash escapes.
 *
 * Returns number of arguments parsed, and updates the tokens[] array
 * with pointers to the arguments, each as an allocated string.
 */
size_t tokenize(char const* line, char* words[]);

/* Find next instance of a parameter within a word. Sets
 * start and end pointers to the start and end of the parameter
 * token.
 */
char param_scan(char const* word, char const** start, char const** end);

/* Simple string-builder function. Builds up a base
 * string by appending supplied strings/character ranges
 * to it.
 */
char* build_str(char const* start, char const* end);

/* Expands all instances of $! $$ $? and ${param} in a string
 * Returns a newly allocated string that the caller must free
 */
char* expand(char const* word);

/* Checks if a token is a redirect operator.
 * If yes, returns a pointer to a Redirect struct with the appropriate type,
 * otherwise returns NULL
 */
struct redirect* check_redirect(const char* token);

/* Parses an array of tokenized strings and returns a Command struct */
struct command parse_command(char** tokens, size_t n_tokens);

#endif //SMALLSH_PARSER_H