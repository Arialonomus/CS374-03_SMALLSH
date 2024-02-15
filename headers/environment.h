/* Program: CS 374 Assignment 03 - SMALLSH
 *
 * This file contains functions related to the maintenance of regularly
 * updated environment variables
 */

#ifndef SMALLSH_ENVIRONMENT_H
#define SMALLSH_ENVIRONMENT_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Sets the value of the $! environment variable, specific errors are not logged */
int set_bgpid(pid_t pid);

/* Sets the value of the $? environment variable, specific errors are not logged */
int set_exitstatus(int status);

/* Wrapper for getenv() that returns an empty string if no environment variable exists */
char* getenvstr(char const* parameter);

#endif //SMALLSH_ENVIRONMENT_H