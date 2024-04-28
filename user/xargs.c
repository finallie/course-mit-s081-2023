#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int readline(char *new_argv[], int curr_argc)
{
    static char buf[1024];
    int n = 0;
    while (read(0, buf + n, 1))
    {
        if (n == 1023)
        {
            fprintf(2, "argument is too long\n");
            exit(1);
        }
        if (buf[n] == '\n')
        {
            break;
        }
        n++;
    }
    buf[n] = 0;
    if (n == 0)
        return 0;
    int offset = 0;
    while (offset < n)
    {
        new_argv[curr_argc++] = buf + offset;
        while (buf[offset] != ' ' && offset < n)
        {
            offset++;
        }
        while (buf[offset] == ' ' && offset < n)
        {
            buf[offset++] = 0;
        }
    }
    return curr_argc;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(2, "Usage: xargs <command> [args...]\n");
        exit(1);
    }
    char *cmd = argv[1];
    char *args[MAXARG];
    for (int i = 1; i < argc; i++)
    {
        args[i - 1] = argv[i];
    }
    int curr_argc = argc - 1;
    while (readline(args, curr_argc) > 0)
    {
        if (fork() == 0)
        {
            exec(cmd, args);
            fprintf(2, "xargs: exec %s failed\n", cmd);
            exit(1);
        }
        wait(0);
    }

    exit(0);
}