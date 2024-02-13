#include "environment.h"

int set_bgpid(pid_t pid)
{
    char* temp_str = NULL;
    if(asprintf(&temp_str, "%jd", (intmax_t)pid) == -1) return -1;
    if(setenv(expand("$!"), temp_str, 1) == -1) return -1;
    free(temp_str);

    return 0;
}

int set_exitstatus(int status)
{
    char* temp_str = NULL;
    if(asprintf(&temp_str, "%d", status) == -1) return -1;
    if(setenv(expand("$?"), temp_str, 1) == -1) return -1;
    free(temp_str);

    return 0;
}