#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>

int main()
{
printf("我是fork前的进程！我的pid是：%d\n", getpid());

pid_t id = fork();

if(id < 0)
{
	perror("fork");
	return 1;
}	
else if(id == 0)
{
	// child 
	while(1)
	{
		sleep(1);
		printf("我是一个子进程！我的pid：%d，我的父进程id：%d\n",getpid(), getppid());
	}
}
else
{
	// father
	while(1)
	{
		sleep(1);
		printf("我是父进程！我的pid：%d，我的父进程id：%d\n",getpid(), getppid());
	}
}
    return 0;
}
