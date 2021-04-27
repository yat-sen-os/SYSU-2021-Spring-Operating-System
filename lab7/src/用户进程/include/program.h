#ifndef PROGRAM_H
#define PROGRAM_H

#include "list.h"
#include "thread.h"

class ProgramManager
{
public:
    List allPrograms;        // 所有状态的线程/进程的队列
    List readyPrograms;      // 处于ready(就绪态)的线程/进程的队列
    PCB *running;            // 当前执行的线程
    int USER_CODE_SELECTOR;  // 用户代码段选择子
    int USER_DATA_SELECTOR;  // 用户数据段选择子
    int USER_STACK_SELECTOR; // 用户栈段选择子

public:
    ProgramManager();
    void initialize();
    // 创建一个线程并放入就绪队列
    // 成功，返回pid；失败，返回-1
    int executeThread(ThreadFunction function, void *parameter, const char *name, int priority);
    // 执行一次调度
    void schedule();
    // 创建用户进程
    int executeProcess(const char *filename, int priority);

public:
    //  分配一个未被占用的pid
    int allocatePid();
    // 按pid查找线程/进程
    PCB *findProgramByPid(int pid);
    // ListItem转换成PCB
    PCB *ListItem2PCB(ListItem *item);
    // 初始化TSS
    void initializeTSS();
    // 激活线程或进程页目录表
    void activateProgramPage(PCB *program);
    // 更新TSS的ESP0
    void updateESP0(PCB *process);
    // 创建用户虚拟地址池
    // 成功，返回true，失败，返回false
    bool createUserVirtualPool(PCB *process);
    // 创建进程页目录表
    // 成功，返回页目录表地址，失败，返回0
    int createProcessPageDirectory();
};

void program_exit();
void load_process(const char *filename);

#endif