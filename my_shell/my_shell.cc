#include <iostream>
#include <ctype.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <unordered_map>
#include <sys/stat.h>
#include <fcntl.h>

#define COMMAND_SIZE 1024                     // 定义命令行缓冲区大小
#define FORMAT "[%s@%s %s]# "                   // 定义命令行提示符格式

// 下面是shell定义的全局数据

// 1. 命令行参数表，用于存储分解后的命令和参数
#define MAXARGC 128
char *g_argv[MAXARGC];
int g_argc = 0; 

// 2. 环境变量表
#define MAX_ENVS 100
char *g_env[MAX_ENVS];
int g_envs = 0;

// 3. 别名映射表（目前未完全实现，仅做预留）
std::unordered_map<std::string, std::string> alias_list;

// 4. 关于重定向，我们关心的内容，定义重定向的三种类型
#define NONE_REDIR 0      // 无重定向
#define INPUT_REDIR 1     // 输入重定向：使用符号 '<'
#define OUTPUT_REDIR 2    // 输出重定向：使用符号 '>'（覆盖）
#define APPEND_REDIR 3    // 输出重定向：使用符号 '>>'（追加）

// 全局变量，保存重定向类型和重定向文件名
int redir = NONE_REDIR;
std::string filename;

// 用于测试当前工作目录和环境变量的临时缓冲区
char cwd[1024];
char cwdenv[1024];

// 记录上一个命令的退出状态
int lastcode = 0;

// 获取当前用户名称
const char *GetUserName()
{
    const char *name = getenv("USER");
    return name == NULL ? "None" : name;
}

// 获取主机名称
const char *GetHostName()
{
    const char *hostname = getenv("HOSTNAME");
    return hostname == NULL ? "None" : hostname;
}

// 获取当前工作目录，并更新环境变量PWD
const char *GetPwd()
{
    // 使用getcwd获取当前工作目录
    const char *pwd = getcwd(cwd, sizeof(cwd));
    if(pwd != NULL)
    {
        // 将当前目录构造为PWD环境变量的格式
        snprintf(cwdenv, sizeof(cwdenv), "PWD=%s", cwd);
        putenv(cwdenv);
    }
    return pwd == NULL ? "None" : pwd;
}

// 获取当前用户主目录
const char *GetHome()
{
    const char *home = getenv("HOME");
    return home == NULL ? "" : home;
}

// 初始化环境变量，复制系统环境变量到全局环境变量数组中，并额外添加一个测试变量
void InitEnv()
{
    extern char **environ;
    memset(g_env, 0, sizeof(g_env));
    g_envs = 0;

    // 1. 从系统中获取环境变量，每个变量复制一份存储到g_env中
    for(int i = 0; environ[i]; i++)
    {
        // 申请空间并复制环境变量字符串
        g_env[i] = (char*)malloc(strlen(environ[i])+1);
        strcpy(g_env[i], environ[i]);
        g_envs++;
    }
    // 添加一个自定义的测试环境变量
    g_env[g_envs++] = (char*)"HAHA=for_test"; // for_test
    g_env[g_envs] = NULL;

    // 2. 将所有环境变量通过putenv注册到系统中
    for(int i = 0; g_env[i]; i++)
    {
        putenv(g_env[i]);
    }
    environ = g_env;
}

// 内置命令cd的实现，用于改变当前工作目录
bool Cd()
{
    // 当只有cd一个参数时，返回到主目录
    if(g_argc == 1)
    {
        std::string home = GetHome();
        if(home.empty()) return true;
        chdir(home.c_str());
    }
    else
    {
        std::string where = g_argv[1];
        // 处理 cd - 或 cd ~ 的情况，目前未完全实现（注释中写到“Todu”表示待实现）
        if(where == "-")
        {
            // 待实现：切换到上一个目录
        }
        else if(where == "~")
        {
            // 待实现：切换到用户主目录
        }
        else
        {
            // 直接切换到指定目录
            chdir(where.c_str());
        }
    }
    return true;
}

// 内置命令echo的实现，用于打印参数或环境变量
void Echo()
{
    if(g_argc == 2)
    {
        std::string opt = g_argv[1];
        // 如果打印"$?"，则输出上一个命令的退出码
        if(opt == "$?")
        {
            std::cout << lastcode << std::endl;
            lastcode = 0;
        }
        // 如果参数以'$'开头，认为是环境变量的引用
        else if(opt[0] == '$')
        {
            std::string env_name = opt.substr(1);
            const char *env_value = getenv(env_name.c_str());
            if(env_value)
                std::cout << env_value << std::endl;
        }
        else
        {
            // 否则直接打印参数内容
            std::cout << opt << std::endl;
        }
    }
}

// 获取当前目录的最后一部分名称（类似basename函数）
// 例如传入"/a/b/c"，返回"c"
std::string DirName(const char *pwd)
{
#define SLASH "/"
    std::string dir = pwd;
    if(dir == SLASH) return SLASH;
    auto pos = dir.rfind(SLASH);
    if(pos == std::string::npos) return "BUG?";
    return dir.substr(pos+1);
}

// 构造命令行提示符字符串
void MakeCommandLine(char cmd_prompt[], int size)
{
    // 使用格式化字符串，将用户名、主机名和当前目录最后一部分组合到提示符中
    snprintf(cmd_prompt, size, FORMAT, GetUserName(), GetHostName(), DirName(GetPwd()).c_str());
}

// 打印命令行提示符
void PrintCommandPrompt()
{
    char prompt[COMMAND_SIZE];
    MakeCommandLine(prompt, sizeof(prompt));
    printf("%s", prompt);
    fflush(stdout);
}

// 从标准输入获取用户输入的命令行，并去除末尾的换行符
bool GetCommandLine(char *out, int size)
{
    // fgets读取一行输入，例如 "ls -a -l\n"
    char *c = fgets(out, size, stdin);
    if(c == NULL) return false;
    out[strlen(out)-1] = 0; // 将末尾的'\n'字符置为'\0'
    if(strlen(out) == 0) return false;
    return true;
}

// 命令行分析函数：将输入字符串根据空格分割成命令和参数
bool CommandParse(char *commandline)
{
#define SEP " "
    g_argc = 0;
    // 使用strtok以空格为分隔符，将命令行分割为多个token
    g_argv[g_argc++] = strtok(commandline, SEP);
    while((bool)(g_argv[g_argc++] = strtok(nullptr, SEP)));
    g_argc--; // 最后一个token为NULL，不计入参数个数
    return g_argc > 0 ? true:false;
}

// 打印解析后的参数列表（调试用）
void PrintArgv()
{
    for(int i = 0; g_argv[i]; i++)
    {
        printf("argv[%d]->%s\n", i, g_argv[i]);
    }
    printf("argc: %d\n", g_argc);
}

// 检查并执行内置命令（如cd、echo等）
// 如果命令是内置命令，则在当前进程中直接执行，不需要fork子进程
bool CheckAndExecBuiltin()
{
    std::string cmd = g_argv[0];
    if(cmd == "cd")
    {
        Cd();
        return true;
    }
    else if(cmd == "echo")
    {
        Echo();
        return true;
    }
    else if(cmd == "export")
    {
        // 待实现：设置环境变量
    }
    else if(cmd == "alias")
    {
       // 待实现：添加别名映射
    }

    return false;
}

// 执行外部命令，主要通过fork创建子进程，再调用execvp进行程序替换
int Execute()
{
    pid_t id = fork();
    if(id == 0)
    {
        int fd = -1;
        // 在子进程中根据redirection标志进行重定向设置
        if(redir == INPUT_REDIR)
        {
            // 输入重定向：打开指定文件（只读模式）
            fd = open(filename.c_str(), O_RDONLY);
            if(fd < 0) exit(1);
            // 将标准输入（文件描述符0）重定向到打开的文件
            dup2(fd, 0);
            close(fd);
        }
        else if(redir == OUTPUT_REDIR)
        {
            // 输出重定向（覆盖）：以写入、创建、截断模式打开文件
            fd = open(filename.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
            if(fd < 0) exit(2);
            // 将标准输出（文件描述符1）重定向到打开的文件
            dup2(fd, 1);
            close(fd);
        }
        else if(redir == APPEND_REDIR)
        {
            // 输出重定向（追加）：以写入、创建、追加模式打开文件
            fd = open(filename.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0666);
            if(fd < 0) exit(2);
            // 将标准输出重定向到文件，以便追加内容
            dup2(fd, 1);
            close(fd);
        }
        else
        {
            // 无重定向时，保持原有标准输入/输出
        }
        // 使用execvp进行进程替换，执行外部命令
        execvp(g_argv[0], g_argv);
        exit(1);  // 如果execvp失败，则退出子进程
    }
    int status = 0;
    // 父进程等待子进程执行完毕，并获取其退出状态
    pid_t rid = waitpid(id, &status, 0);
    if(rid > 0)
    {
        lastcode = WEXITSTATUS(status);
    }
    return 0;
}

/*
 * 函数：TrimSpace
 * 功能：跳过命令字符串中连续的空格字符
 * 参数：cmd[]为命令字符串，end为当前索引位置（传入引用，修改后传出新的索引）
 */
void TrimSpace(char cmd[], int &end)
{
    while(isspace(cmd[end]))
    {
        end++;
    }
}

/*
 * 函数：RedirCheck
 * 功能：检查命令行字符串中是否存在重定向符号，并解析出重定向的类型和目标文件名
 * 实现思路：
 *   - 从命令行字符串的末尾向前扫描，查找'<', '>'或">>"符号。
 *   - 当找到重定向符号时，将该符号及其后的部分截断为文件名字符串，同时设置全局变量redir和filename。
 *   - 注意：该实现假定重定向符号出现在命令的末尾部分。
 */
void RedirCheck(char cmd[])
{
    redir = NONE_REDIR;      // 初始化重定向类型为无重定向
    filename.clear();        // 清空文件名字符串
    int start = 0;
    int end = strlen(cmd)-1; // 从字符串末尾开始扫描

    // 从命令末尾向前查找重定向符号
    while(end > start)
    {
        if(cmd[end] == '<')
        {
            // 找到输入重定向符号 '<'
            cmd[end++] = 0;     // 将'<'位置设置为字符串结束符，截断命令字符串
            TrimSpace(cmd, end);// 跳过重定向符号后的空格
            redir = INPUT_REDIR;
            filename = cmd+end; // 剩余部分作为目标文件名
            break;
        }
        else if(cmd[end] == '>')
        {
            // 判断是否为追加重定向 ">>"
            if(cmd[end-1] == '>')
            {
                // 追加重定向 ">>"
                cmd[end-1] = 0; // 截断字符串，去掉前面的一个'>'
                redir = APPEND_REDIR;
            }
            else
            {
                // 普通输出重定向 ">"
                redir = OUTPUT_REDIR;
            }
            cmd[end++] = 0;     // 将'>'位置设置为结束符，截断命令字符串
            TrimSpace(cmd, end);// 跳过重定向符号后的空格
            filename = cmd+end; // 剩余部分作为目标文件名
            break;
        }
        else
        {
            // 若当前字符不是重定向符号，则继续向前扫描
            end--;
        }
    }
}

int main()
{
    // shell启动时，从系统中获取环境变量，并初始化全局环境变量表
    InitEnv();

    while(true)
    {
        // 1. 输出命令行提示符
        PrintCommandPrompt();

        // 2. 获取用户输入的命令
        char commandline[COMMAND_SIZE];
        if(!GetCommandLine(commandline, sizeof(commandline)))
            continue;

        // 3. 分析重定向情况
        //    例如："ls -a -l > file.txt" 将被拆分为实际命令和重定向目标文件
        RedirCheck(commandline);

        // 调试时可打印重定向状态：redir和filename
        // printf("redir: %d, filename: %s\n", redir, filename.c_str());

        // 4. 命令行解析：将命令字符串分割为命令及其参数数组
        if(!CommandParse(commandline))
            continue;
        // PrintArgv(); // 可用于调试参数解析结果

        // 检查并处理别名（暂未实现具体逻辑）

        // 5. 检查是否为内置命令，如果是则直接执行，不再fork子进程
        if(CheckAndExecBuiltin())
            continue;

        // 6. 如果不是内置命令，则通过fork创建子进程执行外部命令
        Execute();
    }
    // 程序结束前可以在此处进行清理工作（例如释放内存），目前未实现
    return 0;
}
