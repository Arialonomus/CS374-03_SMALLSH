/* Program: CS 374 Assignment 03 - SMALLSH
 *
 * This file contains signal and exit handlers and any related functions
 */

#ifndef SMALLSH_HANDLERS_H
#define SMALLSH_HANDLERS_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <signal.h>
#include <stdlib.h>
#include <err.h>
#include <stdio.h>
#include "environment.h"


/* Global variables for signal handling */
static const int IGNORED[] = {SIGINT, SIGTSTP};
static const size_t NUM_IGNORED = sizeof(IGNORED) / sizeof(int);

typedef void (*sighandler_t)(int);

/* Empty handler for SIG_INT, used to safely restart line reading in interactive mode */
void sigint_handler(int sig);

/* Sends a SIGCONT signal to a stopped child and prints a notice of this action to stderror */
void continue_child(pid_t pid);

#endif //SMALLSH_HANDLERS_H