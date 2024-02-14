/*
 * Program: CS 374 Assignment 03 - SMALLSH
 *
 * This file contains functions related to parameter expansion in the shell
 *
 * Skeleton code provided by OSU Instructor
 */

#ifndef SMALLSH_EXPAND_H
#define SMALLSH_EXPAND_H

#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

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

/* Wrapper for getenv() that returns an empty string if no environment variable exists */
char* getenvstr(char const* parameter);

/* Expands all instances of $! $$ $? and ${param} in a string
 * Returns a newly allocated string that the caller must free
 */
char* expand(char const *word);

#endif //SMALLSH_EXPAND_H
