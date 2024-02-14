/*
 * Program: CS 374 Assignment 03 - SMALLSH
 *
 * This file contains functions related to command execution
 */

#ifndef SMALLSH_COMMAND_H
#define SMALLSH_COMMAND_H

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
enum redirect_t
{
    IN,
    OUT,
    APPEND
};

/* Holds information about a redirect for a command */
struct Redirect
{
    enum redirect_t type;   // The type of redirect
    char* destination;      // The filepath destination of the redirect
};

/* Flags for determining execution for pre-build commands */
enum CommandType
{
    CD,
    EXIT,
    EXTERNAL
};

/* Holds data about a command to be executed */
struct Command
{
    enum CommandType cmd_t;         // Flags the type of the command
    char* name;                     // The name of the command to execute
    char** argv;                    // The list of argument strings
    size_t argc;                    // Argument count (incl. cmd name at argv[0])
    struct Redirect** redirects;    // The redirects for this command
    size_t rd_count;                // Number of redirects
    bool background;                // Flag for if process should run in background
};

/* Built-In Command "cd": changes the working directory of smallsh */
void cmd_cd (char** argv, const int argc);

/* Built-In Command "exit": safely exits smallsh */
void cmd_exit(char** argv, const int argc);

/* Handles process forking for external commands */
void cmd_external(struct Command cmd, struct sigaction* dispositions[]);

/* Sends a SIGCONT signal to a stopped child and prints a notice of this action to stderror */
void continue_child(pid_t pid);

/* Executes a program based on a passed-in command */
void execute(struct Command cmd);

#endif //SMALLSH_COMMAND_H