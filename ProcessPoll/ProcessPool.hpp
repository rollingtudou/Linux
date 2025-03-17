#ifndef __PROCESS_POOL_HPP__
#define __PROCESS_POOL_HPP__

#include <iostream>
#include <cstdlib>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include "Task.hpp"

// Channel类：表示父子进程间的通信通道
// 每个Channel对象包含一个写文件描述符和对应的子进程ID
class Channel
{
public:
    // 构造函数：初始化写文件描述符和子进程ID
    Channel(int fd, pid_t id) : _wfd(fd), _subid(id)
    {
        _name = "channel-" + std::to_string(_wfd) + "-" + std::to_string(_subid);
    }
    ~Channel()
    {
    }
    // 向子进程发送任务码
    void Send(int code)
    {
        int n = write(_wfd, &code, sizeof(code));
        (void)n; // 防止编译器警告未使用的变量
    }
    // 关闭通信管道
    void Close()
    {
        close(_wfd);
    }
    // 等待子进程结束
    void Wait()
    {
        pid_t rid = waitpid(_subid, nullptr, 0);
        (void)rid;
    }
    // 获取文件描述符
    int Fd() { return _wfd; }
    // 获取子进程ID
    pid_t SubId() { return _subid; }
    // 获取通道名称
    std::string Name() { return _name; }

private:
    int _wfd;          // 写端文件描述符
    pid_t _subid;      // 子进程ID
    std::string _name; // 通道名称，用于调试
};

// ChannelManager类：管理所有的通信通道
// 实现负载均衡的任务分发
class ChannelManager
{
public:
    ChannelManager() : _next(0)
    {
    }
    // 添加新的通信通道
    void Insert(int wfd, pid_t subid)
    {
        _channels.emplace_back(wfd, subid);
    }
    // 轮询选择下一个可用的通道
    Channel &Select()
    {
        auto &c = _channels[_next];
        _next++;
        _next %= _channels.size(); // 循环选择
        return c;
    }
    // 打印所有通道信息（调试用）
    void PrintChannel()
    {
        for (auto &channel : _channels)
        {
            std::cout << channel.Name() << std::endl;
        }
    }
    // 停止所有子进程：关闭所有通信管道
    void StopSubProcess()
    {
        for (auto &channel : _channels)
        {
            channel.Close();
            std::cout << "关闭: " << channel.Name() << std::endl;
        }
    }
    // 等待所有子进程结束
    void WaitSubProcess()
    {
        for (auto &channel : _channels)
        {
            channel.Wait();
            std::cout << "回收: " << channel.Name() << std::endl;
        }
    }
    ~ChannelManager() {}

private:
    std::vector<Channel> _channels; // 存储所有通信通道
    int _next;                      // 下一个要使用的通道索引
                                    // 用于负载均衡 轮番使用进程池中的进程
};

// 默认进程池大小
const int gdefaultnum = 5;

// ProcessPool类：进程池的主要实现
class ProcessPool
{
public:
    // 构造函数：初始化进程池，注册默认任务
    ProcessPool(int num) : _process_num(num)
    {
        _tm.Register(PrintLog);
        _tm.Register(Download);
        _tm.Register(Upload);
    }

    // 子进程的工作循环
    // rfd -> pipefd[0]：读端文件的描述符
    void Work(int rfd)
    {
        while (true)
        {
            int code = 0;
            // 如果父进程没有发送任务码，阻塞等待，不会触发管道关闭
            ssize_t n = read(rfd, &code, sizeof(code));
            if (n > 0)
            {
                if (n != sizeof(code))
                {
                    continue;
                }
                std::cout << "子进程[" << getpid() << "]收到一个任务码: " << code << std::endl;
                _tm.Execute(code);
            }
            else if (n == 0) // 管道被关闭
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

    // 启动进程池：创建子进程和通信管道
    bool Start()
    {
        for (int i = 0; i < _process_num; i++)
        {
            // 1. 创建管道
            int pipefd[2] = {0};
            int n = pipe(pipefd);
            if (n < 0)
                return false;

            // 2. 创建子进程
            pid_t subid = fork();
            if (subid < 0)
                return false;
            else if (subid == 0)
            {
                // 子进程：关闭写端，保留读端
                close(pipefd[1]);
                Work(pipefd[0]); // 开始工作循环
                close(pipefd[0]);
                exit(0);
            }
            else
            {
                // 父进程：关闭读端，保留写端
                close(pipefd[0]);
                _cm.Insert(pipefd[1], subid);
            }
        }
        return true;
    }

    // 打印调试信息
    void Debug()
    {
        _cm.PrintChannel();
    }

    // 运行任务：选择任务和子进程，发送任务
    void Run()
    {
        // 1. 获取一个任务码
        int taskcode = _tm.Code();

        // 2. 选择一个子进程执行任务（负载均衡）
        auto &c = _cm.Select();
        std::cout << "选择了一个子进程: " << c.Name() << std::endl;
        // 3. 发送任务
        c.Send(taskcode);
        std::cout << "发送了一个任务码: " << taskcode << std::endl;
    }

    // 停止进程池：关闭所有通信管道，回收子进程
    void Stop()
    {
        _cm.StopSubProcess(); // 关闭所有通信管道

        // 现在子进程为 Z，回收子进程
        _cm.WaitSubProcess(); // 等待所有子进程结束
    }

    ~ProcessPool()
    {
    }

private:
    ChannelManager _cm; // 通道管理器
    int _process_num;   // 进程池大小
    TaskManager _tm;    // 任务管理器
};

#endif