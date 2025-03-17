#pragma once

#include <iostream>
#include <vector>
#include <ctime>

// 定义函数指针类型，用于存储任务处理函数
typedef void (*task_t)();

//////////////// 示例任务函数 /////////////////////
// 打印日志任务：模拟日志记录功能
void PrintLog()
{
    std::cout << "我是一个打印日志的任务" << std::endl;
}

// 下载任务：模拟文件下载功能
void Download()
{
    std::cout << "我是一个下载的任务" << std::endl;
}

// 上传任务：模拟文件上传功能
void Upload()
{
    std::cout << "我是一个上传的任务" << std::endl;
}
//////////////////////////////////////

// TaskManager类：管理和执行任务的类
class TaskManager
{
public:
    // 构造函数：初始化随机数生成器
    TaskManager()
    {
        srand(time(nullptr));
    }

    // 注册新任务到任务列表
    // @param t: 任务函数指针
    void Register(task_t t)
    {
        _tasks.push_back(t);
    }

    // 随机生成任务码
    // @return: 返回一个随机的任务索引
    int Code()
    {
        return rand() % _tasks.size();
    }

    // 执行指定编号的任务
    // @param code: 任务编号
    void Execute(int code)
    {
        if(code >= 0 && code < _tasks.size())
        {
            _tasks[code]();
        }
    }

    ~TaskManager()
    {}
private:
    std::vector<task_t> _tasks;  // 存储所有注册的任务函数
};