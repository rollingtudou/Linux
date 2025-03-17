#include <stdio.h>
#include <unistd.h>

int gval = 100;

int main()
{
    pid_t id = fork();
    if (id == 0)
    {
        while (1)
        {
            printf("子: gval: %d, &gval: %p, pid: %d, ppid: %d\n", gval, &gval, getpid(), getppid());
            sleep(1);
            gval++;
        }
    }
    else
    {
        while (1)
        {
            printf("父: gval: %d, &gval: %p, pid: %d, ppid: %d\n", gval, &gval, getpid(), getppid());
            sleep(1);
        }
    }
    return 0;
}
