#include "environment.h"

int set_bgpid(pid_t pid)
{
    char* temp_str = NULL;
    if(asprintf(&temp_str, "%jd", (intmax_t)pid) == -1) return -1;
    if(setenv("!", temp_str, 1) == -1) return -1;
    free(temp_str);

    return 0;
}

int set_exitstatus(int status)
{
    char* temp_str = NULL;
    if(asprintf(&temp_str, "%d", status) == -1) return -1;
    if(setenv("?", temp_str, 1) == -1) return -1;
    free(temp_str);

    return 0;
}

char* getenvstr(char const* parameter)
{
    char* ret_str = getenv(parameter);
    if (ret_str == NULL) return "";
    return ret_str;
}