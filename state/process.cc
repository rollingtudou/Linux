#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

int main()
{
    printf("我是fork前的进程！我的pid是：%d\n", getpid());

    pid_t id = fork();

    if (id < 0)
    {
        perror("fork");
        return 1;
    }
    else if (id == 0)
    {
        // child process
        int i = 5;
        while (i--)
        {
            sleep(1);
        }
        printf("子进程即将结束，我将变成僵尸进程。\n");
        // 子进程在这里结束，但父进程不会等待它，所以它会变成僵尸进程。
    }
    else
    {
        // father process
        // 父进程进入无限循环，不调用wait或waitpid
        while (1)
        {
            sleep(1);
            printf("我是父进程！我的pid：%d，我的父进程id：%d\n", getpid(), getppid());
        }
    }

    return 0;
}
