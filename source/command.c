#include "command.h"

void cmd_cd (char** argv, const int argc)
{
    if (argc > 2) {
        errno = E2BIG;
        warn("cd");
        return;
    }
    if (argc == 1) {
        argv[1] = getenv("HOME");
    }
    if(chdir(argv[1]) != 0) warn("chdir");
}

void cmd_exit(char** argv, const int argc)
{
    /* Validate command args */
    if (argc > 2) {
        errno = E2BIG;
        warn("exit");
        return;
    }
    if (argc == 1) {
        argv[1] = getenvstr("?");
    }

    /* Convert argument to status int */
    errno = 0;
    char* end = NULL;
    const long status = strtol(argv[1], &end, 10);
    if(strcmp(argv[1], end) == 0 || status > 255 || status < 0) {
        warnx("exit: Invalid argument");
        return;
    }
    if(errno != 0) {
        warn("exit");
        return;
    }
    _exit(status);
}

void cmd_external(struct Command cmd, struct sigaction* dispositions[])
{
    pid_t child_pid;
    switch(child_pid = fork()) {
        /* Error */
        case -1:
            warn("fork");
            break;
        /* Child Process */
        case 0:
            /* Reset signal dispositions */
            for (int i = 0; i < NUM_IGNORED; ++i) {
                if(sigaction(IGNORED[i], dispositions[i], NULL) == -1)
                    err(1, "sigaction(): restore disposition for %d", IGNORED[i]);
            }
            /* Execute external command */
            execute(cmd);
            break;
        /* Parent Process */
        default:
            /* Foreground Processes */
            if (!cmd.background) {
                int status;
                if(waitpid(child_pid, &status, 0) == -1)
                    err(1, "waitpid(): %jd", (intmax_t)child_pid);
                if (WIFEXITED(status)) {
                    if (set_exitstatus(WEXITSTATUS(status)) == -1) err(1, "set_exitstatus()");
                }
                else if (WIFSIGNALED(status)) {
                    int termSig = WTERMSIG(status) + TERMSIG_OFFSET;
                    if (set_exitstatus(termSig) == -1) err(1, "set_exitstatus()");
                }
                else if (WIFSTOPPED(status)) {
                    continue_child(child_pid);
                }
            }
            /* Background Processes */
            else {
                if (set_bgpid(child_pid) == -1) err(1, "set_bgpid()");
            }
            break;
    }
}

void continue_child(pid_t pid)
{
    kill(pid, SIGCONT);
    fprintf(stderr, "Child process %jd stopped. Continuing.\n", (intmax_t) pid);
    if (set_bgpid(pid) == -1) err(1, "set_bgpid()");
}

void execute(struct Command cmd)
{
    /* Handle redirection */
    if(cmd.inputFile) {
        int input_fd = open(cmd.inputFile, O_RDONLY );
        if (input_fd == -1) err(1, "open: %s", cmd.inputFile);
        close (STDIN_FILENO);
        if (dup2(input_fd, STDIN_FILENO) == -1) err(1, "source dup2()");
        close (input_fd);
    }
    if(cmd.outputFile) {
        int openFlags = cmd.append == true ? O_WRONLY | O_CREAT | O_APPEND : O_WRONLY | O_CREAT | O_TRUNC;
        int output_fd = open(cmd.outputFile, openFlags, 0777);
        if (output_fd == -1) err(1, "open: %s", cmd.outputFile);
        close (STDOUT_FILENO);
        if (dup2(output_fd, STDOUT_FILENO) == -1) err(1, "target dup2()");
        close (output_fd);
    }

    /* Execute external program */
    if(execvp(cmd.name, cmd.argv) == -1) err(1, "execvp(): %s", cmd.name);
}