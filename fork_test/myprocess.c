#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>

int main()
{
    printf("父进程开始执行，pid = %d \n", getpid());

    fork();
    
    printf("进程开始执行，pid = %d \n", getpid());
    return 0;
}
