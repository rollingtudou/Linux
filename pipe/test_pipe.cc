// #include <iostream>
// #include <unistd.h>

// int main()
// {
//     int fd[2];
//     int n = pipe(fd);

//     if(n < 0) std::cout << "pipe error" << std::endl;
    
//     std::cout << "fd[0]" << fd[0] << std::endl;
//     std::cout << "fd[1]" << fd[1] << std::endl;

//     return 0;
// }




// 例⼦：从键盘读取数据，写⼊管道，读取管道，写到屏幕

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    int fds[2];          // 管道文件描述符数组：fds[0] 为读端，fds[1] 为写端
    char buf[100];       // 缓冲区，用于存储输入和输出数据
    int len;             // 记录读取或写入的字节数

    // 创建管道
    if (pipe(fds) == -1) {
        perror("make pipe");
        exit(1);
    }

    // 从标准输入（键盘）读取数据
    while (fgets(buf, 100, stdin)) {
        len = strlen(buf);  // 计算输入数据的长度

        // 将数据写入管道写端
        if (write(fds[1], buf, len) != len) {
            perror("write to pipe");
            break;
        }

        // 清空缓冲区，为读取准备
        memset(buf, 0x00, sizeof(buf));

        // 从管道读端读取数据
        if ((len = read(fds[0], buf, 100)) == -1) {
            perror("read from pipe");
            break;
        }

        // 将读取的数据写入标准输出（屏幕）
        if (write(1, buf, len) != len) {
            perror("write to stdout");
            break;
        }
    }

    return 0;
}