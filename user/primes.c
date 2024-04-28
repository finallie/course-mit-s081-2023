#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void redirect(int k, int p[])
{
    close(k);
    dup(p[k]);
    close(p[0]);
    close(p[1]);
}

void sourse()
{
    for (int i = 2; i <= 35; i++)
    {
        write(1, &i, sizeof(i));
    }
}

void cull(int p)
{
    int n;
    for (;;)
    {
        if (read(0, &n, sizeof(n)) == 0)
        {
            break;
        }
        if (n % p != 0)
        {
            write(1, &n, sizeof(n));
        }
    }
}

void sink()
{
    int pd[2];
    int p;
    for (;;)
    {
        if (read(0, &p, sizeof(p)) == 0)
        {
            break;
        }
        printf("prime %d\n", p);
        pipe(pd);
        if (fork() == 0)
        {
            redirect(1, pd);
            cull(p);
        }
        else
        {
            redirect(0, pd);
        }
    }
}

int main(int argc, char *argv[])
{
    int pd[2];
    pipe(pd);
    if (fork() == 0)
    {
        redirect(1, pd);
        sourse();
    }
    else
    {
        redirect(0, pd);
        sink();
    }
    exit(0);
}