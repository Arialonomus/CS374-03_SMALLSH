/*
 * Program: CS 374 Assignment 03 - SMALLSH
 *
 * This file contains constants, functions, and program-level variables
 * related to tokenizing input for parsing by the shell
 *
 * Skeleton code provided by OSU Instructor
 */

#ifndef SMALLSH_WORDSPLIT_H
#define SMALLSH_WORDSPLIT_H

#include <stdlib.h>
#include <err.h>
#include <ctype.h>

/*
 * Variables
 */

#ifndef MAX_WORDS
#define MAX_WORDS 512
#endif

char* words[MAX_WORDS];  // A program-level array of words representing tokenized arguments

/* Splits a string into words delimited by whitespace. Recognizes
 * comments as '#' at the beginning of a word, and backslash escapes.
 *
 * Returns number of words parsed, and updates the words[] array
 * with pointers to the words, each as an allocated string.
 */
size_t wordsplit(char const *line);

#endif //SMALLSH_WORDSPLIT_H
