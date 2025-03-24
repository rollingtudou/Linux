#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main()
{
    int outfd = open("dest.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644); // 打开目标文件
    if (outfd == -1)
    {
        perror("open dest");
        exit(1);
    }
    int infd = open("fifo", O_RDONLY); // 打开 FIFO 以读取
    if (infd == -1)
    {
        perror("open fifo");
        exit(1);
    }
    char buf[1024];
    int n;
    while ((n = read(infd, buf, 1024)) > 0)
    {
        write(outfd, buf, n); // 写入目标文件
    }
    close(infd);
    close(outfd);
    unlink("fifo"); // 删除 FIFO
    return 0;
}