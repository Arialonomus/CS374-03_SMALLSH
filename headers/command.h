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
enum CommandFlag
{
    CD,
    EXIT,
    EXTERNAL
};

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
    enum CommandFlag flag;  // Tells program whether command is a built-in or external
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

#endif //SMALLSH_COMMAND_H
