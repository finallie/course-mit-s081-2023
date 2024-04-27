#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int p1[2], p2[2];
    uint8 b;
    pipe(p1);
    pipe(p2);
    if (fork() == 0)
    {
        close(p1[1]);
        close(p2[0]);
        if (read(p1[0], &b, 1) != 1)
        {
            fprintf(2, "child: read error\n");
            exit(1);
        }
        printf("%d: received ping\n", getpid());
        if (write(p2[1], &b, 1) != 1)
        {
            fprintf(2, "child: write error\n");
            exit(1);
        }
        exit(0);
    }
    close(p1[0]);
    close(p2[1]);
    if (write(p1[1], &b, 1) != 1)
    {
        fprintf(2, "parent: write error\n");
        exit(1);
    }
    if (read(p2[0], &b, 1) != 1)
    {
        fprintf(2, "parent: read error\n");
        exit(1);
    }
    printf("%d: received pong\n", getpid());
    exit(0);
}