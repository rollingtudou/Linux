#ifndef __PROCESS_POOL_HPP__
#define __PROCESS_POOL_HPP__

#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include "Task.hpp"

// Channel 类 用于表示匿名管道
class Channel
{
public:
    Channel(int fd, pid_t id)
        : _wfd(fd), _subid(id)
    {
        _name = "channel - " + std::to_string(_wfd) + " - " + std::to_string(_subid);
    }

    // 向子进程发送任务码
    void Send(int code)
    {
        int n = write(_wfd, &code, sizeof(code)); // 未理解，发到哪里？怎么用？
        (void)n;
    }

    // 用于在信道管理对象中关闭信道
    void Close()
    {
        close(_wfd);
    }

    // 用于在信道管理对象中等待进程
    void Wait()
    {
        pid_t rid = waitpid(_subid, nullptr, 0);
        (void)rid;
    }

    int Fd() { return _wfd; }
    pid_t Subid() { return _subid; }
    std::string Name() { return _name; }

    ~Channel() {}

private:
    int _wfd;
    pid_t _subid;
    std::string _name;
};

class ChannelManager
{
public:
    ChannelManager() : _next(0) // 进程池中进程使用时，先使用0索引下的进程
    {
    }

    // 将子进程的写端描述符和子进程的pid保存到容器中
    void Insert(int wfd, pid_t subid)
    {
        _channels.emplace_back(wfd, subid);
    }

    // 随机选择一个任务
    Channel &Select()
    {
        auto &c = _channels[_next]; // 从索引0开始，之后都每次指向下一个
        _next++;
        _next %= _channels.size(); // 防止错误的索引

        return c;
    }

    // 关闭所有信道
    void StopSubProcess()
    {
        for (auto &channel : _channels)
        {
            channel.Close();
            std::cout << "关闭 " << channel.Name() << std::endl;
        }
    }

    // 回收所有子进程
    void WaitSubProcess()
    {
        for(auto &channel : _channels)
        {
            channel.Wait();
            std::cout << "回收：" << channel.Subid() << std::endl;
        }
    }

    ~ChannelManager() {}

private:
    std::vector<Channel> _channels;
    int _next; // 下一个要使用的通道索引
};

class ProcessPool
{
public:
    ProcessPool(int num) : _process_num(num)
    {
        _tm.Register(PrintLog);
        _tm.Register(Download);
        _tm.Register(Upload);
    }

    // 子进程的工作循环，等待父进程发送任务码，执行任务
    void Work(int rfd)
    {
        while (true)
        {
            int code = 0;
            // 如果父进程没有发送任务码，read 阻塞等待，不会触发管道关闭
            ssize_t n = read(rfd, &code, sizeof(code));
            if (n > 0)
            {
                // 如果一次读入的字节数小于sizeof(int)，说明没有读取完整个int，继续读取
                if (n != sizeof(int))
                {
                    continue;
                }
                std::cout << "子进程[" << getpid() << "]收到一个任务码: " << code << std::endl;
                _tm.Execute(code); // 执行任务  还未实现
            }
            else if (n == 0) // 管道被关闭,read 的时候会返回 0
            {
                std::cout << "子进程退出" << std::endl;
                break;
            }
            else
            {
                std::cout << "读取错误" << std::endl;
                break;
            }
        }
    }

    bool Start()
    {
        for (int i = 0; i < _process_num; i++)
        {
            int pipefd[2];
            int n = pipe(pipefd);
            if (n < 0)
                return false;

            pid_t subpid = fork();
            if (subpid < 0)
                return false;
            else if (subpid == 0) // 子进程
            {
                // 先关闭写端，保留读端
                close(pipefd[1]);
                Work(pipefd[0]);  // 子进程的工作函数
                close(pipefd[0]); // 结束任务后将读端也关闭
                exit(0);          // 退出子进程
            }
            else // 父进程
            {
                close(pipefd[0]);
                _cm.Insert(pipefd[1], subpid); // 将当前管道的写端和子进程的pid保存到容器中
            }
        }

        return true;
    }

    void Run()
    {
        // 1. 获取一个任务码
        int taskcode = _tm.Code();

        // 2. 选择一个子进程执行任务（负载均衡）
        auto &c = _cm.Select();
        std::cout << "选择了一个子进程: " << c.Name() << std::endl;

        // 3. 发送任务
        c.Send(taskcode);
        std::cout << "发送了一个任务码：" << taskcode << std::endl;
    }

    // 停止进程池：关闭所有通信通道，回收子进程
    void Stop()
    {
        // 先关闭信道
        _cm.StopSubProcess();

        // 再回收僵尸进程
        _cm.WaitSubProcess();
    }

    ~ProcessPool() {}

private:
    int _process_num;   // 进程池中的进程数
    ChannelManager _cm; // 管道管理器
    TaskManager _tm;    // 任务管理器
};

#endif