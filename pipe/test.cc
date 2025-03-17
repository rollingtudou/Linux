#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>

// 9:
void ChildWrite(int wfd)
{
    char c = 0;
    int cnt = 0;
    while (true)
    {
        //snprintf(buffer, sizeof(buffer), "I am child, pid: %d, cnt: %d", getpid(), cnt++);
        write(wfd, &c, 1);
        printf("child: %d\n", cnt++);
        // sleep(2);
        //break;
        // sleep(3);
    }
}

void FatherRead(int rfd)
{
    char buffer[1024];
    while (true)
    {
        sleep(100);
        buffer[0] = 0;
        ssize_t n = read(rfd, buffer, sizeof(buffer)-1);
        if(n > 0)
        {
            buffer[n] = 0;
            std::cout << "child say: " << buffer << std::endl;
            // sleep(2);
        }
        else if(n == 0)
        {
            std::cout << "n : " << n << std::endl;
            std::cout << "child 退出，我也退出";
            break;
        }
        else
        {
            break;
        }

        break;
    }
}

int main()
{
    // 1. 创建管道
    int fds[2] = {0}; // fds[0]:读端   fds[1]: 写端
    int n = pipe(fds);
    if (n < 0)
    {
        std::cerr << "pipe error" << std::endl;
        return 1;
    }
    std::cout << "fds[0]: " << fds[0] << std::endl;
    std::cout << "fds[1]: " << fds[1] << std::endl;

    // 2. 创建子进程
    pid_t id = fork();
    if (id == 0)
    {
        // child
        // code
        // 3. 关闭不需要的读写端，形成通信信道
        // f -> r, c -> w
        close(fds[0]);
        ChildWrite(fds[1]);
        close(fds[1]);
        exit(0);
    }
    // 3. 关闭不需要的读写端，形成通信信道
    // f -> r, c -> w
    close(fds[1]);
    FatherRead(fds[0]);
    close(fds[0]);

    sleep(5);

    int status = 0;
    int ret = waitpid(id, &status, 0); // 获取到子进程的退出信息吗！！！
    if(ret > 0)
    {
        printf("exit code: %d, exit signal: %d\n", (status>>8)&0xFF, status&0x7F);
        sleep(5);
    }
    return 0;
}