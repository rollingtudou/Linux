#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    umask(0);
    mkfifo("chatfifo", 0666);             // 创建命名管道
    int rfd = open("chatfifo", O_RDONLY); // 以读模式打开
    if (rfd < 0)
    {
        perror("open");
        exit(1);
    }
    char buf[1024];
    while (1)
    {
        printf("等待客户端消息...\n");
        ssize_t s = read(rfd, buf, sizeof(buf) - 1);
        if (s > 0)
        {
            buf[s] = '\0';
            printf("客户端说: %s\n", buf);
        }
        else if (s == 0)
        {
            printf("客户端退出，服务器关闭。\n");
            break;
        }
    }
    close(rfd);
    unlink("chatfifo");
    return 0;
}