#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int n = atoi(argv[1]);
    for (int i = 0; i < 20; i++)
    {
        sleep(n);
        printf("%d\n", uptime());
    }
}