/* Program: CS 374 Assignment 03 - SMALLSH
 *
 * This file contains constants and functions related to parsing a command
 * from lines of input, including tokenizing, parameter expansion, and
 * generating command structures from tokenized input
 *
 * Starter code provided by OSU Instructor for functions: wordsplit(), param_scan()
 * build_str(), and expand().
 */

#ifndef SMALLSH_PARSER_H
#define SMALLSH_PARSER_H

#ifndef MAX_WORDS
#define MAX_WORDS 512
#endif

#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <unistd.h>
#include "environment.h"
#include "command.h"

/* Splits a string into words delimited by whitespace. Recognizes
 * comments as '#' at the beginning of a word, and backslash escapes.
 *
 * Returns number of words parsed, and updates the words[] array
 * with pointers to the words, each as an allocated string.
 */
size_t tokenize(char const* line, char** words);

/* Find next instance of a parameter within a word. Sets
 * start and end pointers to the start and end of the parameter
 * token.
 */
char param_scan(char const *word, char const **start, char const **end);

/* Simple string-builder function. Builds up a base
 * string by appending supplied strings/character ranges
 * to it.
 */
char* build_str(char const *start, char const *end);

/* Expands all instances of $! $$ $? and ${param} in a string
 * Returns a newly allocated string that the caller must free
 */
char* expand(char const *word);

/* Checks if a token is a redirect operator.
 * If yes, returns a pointer to a Redirect struct of the appropriate type,
 * otherwise returns NULL
 */
struct Redirect* checkRedirect(const char* token);

/* Parses an array of tokenized strings and returns a Command struct */
struct Command parseCommand(char** tokens, size_t numTokens);

#endif //SMALLSH_PARSER_H
