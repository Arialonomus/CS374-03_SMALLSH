/* Program: CS 374 Assignment 03 - SMALLSH
 *
 * This file contains signal and exit handlers and any related functions
 */

#ifndef SMALLSH_HANDLERS_H
#define SMALLSH_HANDLERS_H

#include <signal.h>

#define NUM_IGNORED 2

/* Global variables for signal handling */
static const int IGNORED[NUM_IGNORED] = {SIGINT, SIGTSTP};

typedef void (*sighandler_t)(int);

void sigint_handler(int sig);

#endif //SMALLSH_HANDLERS_H