#ifndef TASK_HPP
#define TASK_HPP

#include <iostream>
#include <cstdlib>
#include <vector>
// 定义函数指针类型，用于存储任务处理函数
// task_t 是 void(*)() 的别名
using task_t = void(*)();
// typedef void (*task_t)();  // 另一个写法 但是不能直接表达效果语意
////////////// 示例任务函数 ///////////////////////

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


/////////////////////////////////////////////////////////
// 使用函数指针的原理进行存储函数进行管理调用
class TaskManager
{
public:
    TaskManager() 
    {}

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
    std::vector<task_t> _tasks;
};


#endif