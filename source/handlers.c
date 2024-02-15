#include "handlers.h"

void sigint_handler(int sig)
{

}

void continue_child(pid_t pid)
{
    if(kill(pid, SIGCONT) == -1) err(1, "kill(%jd, SIGCONT)", (intmax_t) pid);
    fprintf(stderr, "Child process %jd stopped. Continuing.\n", (intmax_t) pid);
    if (set_bgpid(pid) == -1) err(1, "set_bgpid()");
}