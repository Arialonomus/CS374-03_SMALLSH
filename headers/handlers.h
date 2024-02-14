/* Program: CS 374 Assignment 03 - SMALLSH
 *
 * This file contains signal and exit handlers and any related functions
 */

#ifndef SMALLSH_HANDLERS_H
#define SMALLSH_HANDLERS_H

#include <signal.h>
#include <stddef.h>

/* Global variables for signal handling */
static const int IGNORED[] = {SIGINT, SIGTSTP};
static const size_t NUM_IGNORED = sizeof(IGNORED) / sizeof(int);

typedef void (*sighandler_t)(int);

void sigint_handler(int sig);

#endif //SMALLSH_HANDLERS_H