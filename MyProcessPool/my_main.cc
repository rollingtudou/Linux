#include "ProcessPool.hpp"

int main()
{
    srand(time(nullptr)); // 后续在为管道派发任务时，使用随机数
    const int gdefaultnum = 5;
    ProcessPool pp(gdefaultnum);

    pp.Start();

    //   模拟派发10个任务，每个任务间隔1秒
    int cnt = 10;
    while (cnt--)
    {
        pp.Run();    // 运行一个随机选择的任务
        sleep(1);    // 等待1秒
    }

    pp.Stop();

    return 0;
}