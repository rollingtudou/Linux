#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main()
{
    mkfifo("fifo", 0644);                    // 创建命名管道
    int infd = open("source.txt", O_RDONLY); // 打开源文件
    if (infd == -1)
    {
        perror("open source");
        exit(1);
    }
    int outfd = open("fifo", O_WRONLY); // 打开 FIFO 以写入
    if (outfd == -1)
    {
        perror("open fifo");
        exit(1);
    }
    char buf[1024];
    int n;

    while ((n = read(infd, buf, 1024)) > 0)
    {
        write(outfd, buf, n); // 写入 FIFO
    }

    close(infd);
    close(outfd);
    return 0;
}