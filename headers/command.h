/*
 * Program: CS 374 Assignment 03 - SMALLSH
 *
 * This file contains functions related to command parsing and execution
 */

#ifndef SMALLSH_COMMAND_H
#define SMALLSH_COMMAND_H

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>

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
    char* command;          // The name of the command to execute
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
enum RD_FLAG checkRedirect(char* token);

#endif //SMALLSH_COMMAND_H
