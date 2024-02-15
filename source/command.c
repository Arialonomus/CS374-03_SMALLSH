#include "command.h"

void cmd_cd (char* argv[], const int argc)
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

void cmd_exit(char* argv[], const int argc)
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
    // Status is not a string, or is too large/too small
    if(strcmp(argv[1], end) == 0 || status > 255 || status < 0) {
        warnx("exit: Invalid argument");
        return;
    }
    // strtol() threw an error
    if(errno != 0) {
        warn("strtol");
        return;
    }
    _exit(status);
}

void cmd_external(const struct command cmd, struct sigaction* dispositions[])
{
    /* Fork a child process */
    pid_t child_pid;
    switch(child_pid = fork()) {
        /* Error */
        case -1:
            warn("fork");
            break;

        /* Child Process */
        case 0:
            /* Reset signal dispositions */
            for (int i = 0; dispositions && i < NUM_IGNORED; ++i) {
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
                if(waitpid(child_pid, &status, WUNTRACED) == -1)
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

void execute(struct command cmd)
{
    /* Handle redirection operations */
    for (int i = 0; i < cmd.rd_count; ++i) {
        /* Set flags and stream target based on redirect type */
        int flags = -1, stream = -1;
        switch (cmd.redirects[i]->type) {
            case IN:
                flags = O_RDONLY;
                stream = STDIN_FILENO;
                break;
            case OUT:
                flags = O_WRONLY | O_CREAT | O_TRUNC;
                stream = STDOUT_FILENO;
                break;
            case APPEND:
                flags = O_WRONLY | O_CREAT | O_APPEND;
                stream = STDOUT_FILENO;
                break;
        }

        /* Point the I/O stream at the destination file */
        char* path = cmd.redirects[i]->destination;
        const int fd = open(path, flags, 0777);
        if (fd == -1) err(1, "open: %s", path);
        close (stream);
        if (dup2(fd, stream) == -1) err(1, "source dup2()");
        close (fd);
        free(cmd.redirects[i]);
    }
    free(cmd.redirects);

    /* Launch external program */
    if(execvp(cmd.name, cmd.argv) == -1) err(1, "execvp(): %s", cmd.name);
}