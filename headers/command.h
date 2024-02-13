/*
 * Program: CS 374 Assignment 03 - SMALLSH
 *
 * This file contains functions related to command parsing and execution
 */

#ifndef SMALLSH_COMMAND_H
#define SMALLSH_COMMAND_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include "handlers.h"
#include "environment.h"

/* Flags for determining execution for pre-build commands */
enum CommandType
{
    CD,
    EXIT,
    EXTERNAL
};

/* Flags for determining redirection type */
enum RD_FLAG
{
    NONE,
    RD_IN,
    RD_OUT,
    RD_APPEND
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

/* Parses an array of tokenized strings and returns a Command struct */
struct Command parseCommand(char** tokens, size_t numTokens);

/* Checks if a token is a redirect operator. If so, returns the operator type */
enum RD_FLAG checkRedirect(const char* token);

/* Built-In Command "cd": changes the working directory of smallsh */
void cmd_cd (char** argv, const int argc);

/* Built-In Command "exit": safely exits smallsh */
void cmd_exit(char** argv, const int argc);

/* Handles process forking for external commands */
void cmd_external(struct Command cmd, struct sigaction* dispositions[]);

/* Executes a program based on a passed-in command */
void execute(struct Command cmd);

#endif //SMALLSH_COMMAND_H