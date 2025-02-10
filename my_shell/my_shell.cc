#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <unordered_map>

// 定义常量
#define COMMAND_SIZE 1024      // 命令缓冲区大小
#define FORMAT "[%s@%s %s]# "  // 命令行提示符格式

/* ========== Shell全局数据结构 ========== */
// 1. 命令行参数表（用于存储解析后的命令参数）
#define MAXARGC 128
char *g_argv[MAXARGC];  // 参数数组
int g_argc = 0;         // 参数个数

// 2. 环境变量表（存储shell的环境变量）
#define MAX_ENVS 100
char *g_env[MAX_ENVS];  // 环境变量数组
int g_envs = 0;         // 环境变量数量

// 3. 别名映射表（用于存储命令别名）
std::unordered_map<std::string, std::string> alias_list;

// 测试用变量
char cwd[1024];     // 当前工作目录缓冲区
char cwdenv[1024];  // 环境变量格式化的PWD

int lastcode = 0;   // 最后退出状态码

/* ========== 环境信息获取函数 ========== */
// 获取当前用户名
const char *GetUserName() {
    const char *name = getenv("USER");
    return name == NULL ? "None" : name;
}

// 获取主机名
const char *GetHostName() {
    const char *hostname = getenv("HOSTNAME");
    return hostname == NULL ? "None" : hostname;
}

// 获取当前工作目录并更新PWD环境变量
const char *GetPwd() {
    const char *pwd = getcwd(cwd, sizeof(cwd));  // 获取真实物理路径
    if(pwd != NULL) {
        // 更新环境变量中的PWD
        snprintf(cwdenv, sizeof(cwdenv), "PWD=%s", cwd);
        putenv(cwdenv);
    }
    return pwd == NULL ? "None" : pwd;
}

// 获取用户家目录
const char *GetHome() {
    const char *home = getenv("HOME");
    return home == NULL ? "" : home;
}

/* ========== 环境初始化 ========== */
void InitEnv() {
    extern char **environ;
    memset(g_env, 0, sizeof(g_env));
    g_envs = 0;

    // 从父进程继承环境变量
    for(int i = 0; environ[i]; i++) {
        g_env[i] = (char*)malloc(strlen(environ[i])+1);
        strcpy(g_env[i], environ[i]);
        g_envs++;
    }
    
    // 添加测试环境变量
    g_env[g_envs++] = (char*)"HAHA=for_test";
    g_env[g_envs] = NULL;

    // 设置全局环境变量
    for(int i = 0; g_env[i]; i++) {
        putenv(g_env[i]);
    }
    environ = g_env;
}

/* ========== 内建命令实现 ========== */
// cd命令处理
bool Cd() {
    if(g_argc == 1) {  // 无参数时切换到家目录
        std::string home = GetHome();
        if(home.empty()) return true;
        chdir(home.c_str());
    } else {
        std::string where = g_argv[1];
        // TODO: 处理特殊参数（- 和 ~）
        chdir(where.c_str());
    }
    return true;
}

// echo命令处理
void Echo() {
    if(g_argc == 2) {
        std::string opt = g_argv[1];
        if(opt == "$?") {  // 显示上一个命令退出码
            std::cout << lastcode << std::endl;
            lastcode = 0;
        } else if(opt[0] == '$') {  // 显示环境变量
            std::string env_name = opt.substr(1);
            const char *env_value = getenv(env_name.c_str());
            if(env_value) std::cout << env_value << std::endl;
        } else {  // 直接输出字符串
            std::cout << opt << std::endl;
        }
    }
}

// 从完整路径中提取目录名
std::string DirName(const char *pwd) {
    std::string dir = pwd;
    if(dir == "/") return "/";
    auto pos = dir.rfind("/");
    return dir.substr(pos+1);
}

/* ========== 命令行界面相关 ========== */
// 生成命令提示符
void MakeCommandLine(char cmd_prompt[], int size) {
    snprintf(cmd_prompt, size, FORMAT, 
             GetUserName(), 
             GetHostName(), 
             DirName(GetPwd()).c_str());
}

// 打印命令提示符
void PrintCommandPrompt() {
    char prompt[COMMAND_SIZE];
    MakeCommandLine(prompt, sizeof(prompt));
    printf("%s", prompt);
    fflush(stdout);
}

// 获取用户输入命令
bool GetCommandLine(char *out, int size) {
    char *c = fgets(out, size, stdin);
    if(c == NULL) return false;
    out[strlen(out)-1] = 0;  // 去掉换行符
    return strlen(out) != 0;
}

/* ========== 命令解析 ========== */
// 解析命令行参数
bool CommandParse(char *commandline) {
#define SEP " "  // 参数分隔符
    g_argc = 0;
    // 使用strtok分割命令行
    g_argv[g_argc++] = strtok(commandline, SEP);
    while((g_argv[g_argc++] = strtok(nullptr, SEP)));
    g_argc--;  // 修正参数个数（最后一个为NULL）
    return g_argc > 0;
}

/* ========== 命令执行 ========== */
// 检查并执行内建命令
bool CheckAndExecBuiltin() {
    std::string cmd = g_argv[0];
    if(cmd == "cd") {
        Cd();
        return true;
    } else if(cmd == "echo") {
        Echo();
        return true;
    } else if(cmd == "export") {
        // TODO: 实现环境变量导出
    } else if(cmd == "alias") {
        // TODO: 实现别名功能
    }
    return false;
}

// 执行外部命令
int Execute() {
    pid_t id = fork();
    if(id == 0) {  // 子进程执行命令
        execvp(g_argv[0], g_argv);
        exit(1);  // 执行失败返回1
    }
    
    // 父进程等待子进程结束
    int status = 0;
    waitpid(id, &status, 0);
    lastcode = WEXITSTATUS(status);  // 记录退出状态码
    return 0;
}

/* ========== 主函数 ========== */
int main() {
    InitEnv();  // 初始化环境变量
    
    while(true) {
        PrintCommandPrompt();                  // 显示提示符
        char commandline[COMMAND_SIZE];
        if(!GetCommandLine(commandline, sizeof(commandline))) continue; // 获取输入
        
        if(!CommandParse(commandline)) continue;   // 解析命令
        if(CheckAndExecBuiltin()) continue;        // 执行内建命令
        
        Execute();  // 执行外部命令
    }
    return 0;
}
