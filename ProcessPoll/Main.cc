#include "ProcessPool.hpp"

int main()
{

    // 子进程读取-> r
    // 父进程写入-> w
    
    // 创建进程池对象，使用默认数量(gdefaultnum=5)的工作进程
    ProcessPool pp(gdefaultnum);

    // 启动进程池：创建子进程和通信管道
    pp.Start();

    // 模拟派发10个任务，每个任务间隔1秒
    int cnt = 10;
    while (cnt--)
    {
        pp.Run();    // 运行一个随机选择的任务
        sleep(1);    // 等待1秒
    }

    // 停止进程池：关闭所有通信管道，回收子进程
    pp.Stop();
    return 0;
}

    // 潜在的bug说明：
    // 1. 当子进程异常退出时，父进程可能无法及时感知
    // 2. 父进程可能继续向已经不存在的子进程发送任务
    // 3. 需要添加信号处理机制来处理子进程异常退出的情况
    // 4. 可以考虑添加心跳检测机制确保子进程存活