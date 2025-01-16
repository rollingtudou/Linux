#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>

int main()
{
    pid_t id = fork();

    if(id == 0)
    {
      int n = 4;
      while(n--)
      {
         printf("我是一个进程！ \n");
         sleep(1);
      }
    }
  else
    {
      while(1)
      {
         printf("我是一个进程！\n");
         sleep(1);
      }
    }
    

    return 0;
}
