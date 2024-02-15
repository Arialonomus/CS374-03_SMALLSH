/* Program: CS 374 Assignment 03 - SMALLSH
 *
 * This file contains functions and structs related to command execution,
 * including execution of built-in commands and external programs
 */

#ifndef SMALLSH_COMMAND_H
#define SMALLSH_COMMAND_H

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdbool.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "handlers.h"
#include "environment.h"

#define TERMSIG_OFFSET 128  // Offset for child process terminating signal reporting

/* Flags for determining redirection type */
enum rd_t
{
    IN,
    OUT,
    APPEND
};

/* Holds information about a redirect for a command */
struct redirect
{
    enum rd_t type;         // The type of redirect
    char* destination;      // The filepath destination of the redirect
};

/* Flags for determining execution for pre-build commands */
enum cmd_t
{
    CD,
    EXIT,
    EXTERNAL
};

/* Holds data about a command to be executed */
struct command
{
    enum cmd_t type;                // Flags the type of the command
    char* name;                     // The name of the command to execute
    char** argv;                    // The list of argument strings
    size_t argc;                    // Argument count (incl. cmd name at argv[0])
    struct redirect** redirects;    // The redirects for this command
    size_t rd_count;                // Number of redirects
    bool background;                // Flag for if process should run in background
};

/* Built-In Command "cd": changes the working directory of smallsh */
void cmd_cd (char* argv[], int argc);

/* Built-In Command "exit": safely exits smallsh */
void cmd_exit(char* argv[], int argc);

/* Handles process forking for external commands */
void cmd_external(struct command cmd, struct sigaction* dispositions[]);

/* Executes a program based on a passed-in command */
void execute(struct command cmd);

#endif //SMALLSH_COMMAND_H