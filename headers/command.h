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
    enum CommandType cmd_t; // Flags the type of the command
    char* name;             // The name of the command to execute
    char** argv;            // The list of arguments
    size_t argc;            // Argument count
    char* inputFile;        // Input file for redirection
    char* outputFile;       // Output file for redirection
    bool append;            // Flag for if input is appended
    bool background;        // Flag for if process should run in background
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