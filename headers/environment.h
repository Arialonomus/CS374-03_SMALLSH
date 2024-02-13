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
#include "expand.h"

int set_bgpid(pid_t pid);

int set_exitstatus(int status);

#endif //SMALLSH_ENVIRONMENT_H
