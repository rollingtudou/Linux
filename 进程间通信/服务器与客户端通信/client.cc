#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


int main()
{
    int wfd = open("chatfifo", O_WRONLY); // 以写模式打开
    if (wfd < 0)
    {
        perror("open");
        exit(1);
    }
    char buf[1024];
    while (1)
    {
        printf("请输入消息# ");
        fflush(stdout);
        ssize_t s = read(0, buf, sizeof(buf) - 1); // 从标准输入读取
        if (s > 0)
        {
            buf[s] = '\0';
            write(wfd, buf, strlen(buf)); // 写入 FIFO
        }
    }
    close(wfd);
    return 0;
}