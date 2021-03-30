#ifndef PROGRAM_H
#define PROGRAM_H

#include "list.h"
#include "thread.h"

class ProgramManager
{
public:
    List allPrograms;   // 所有状态的线程/进程的队列
    List readyPrograms; // 处于ready(就绪态)的线程/进程的队列
    PCB *running;       // 当前执行的线程
public:
    ProgramManager();
    void initialize();
    // 创建一个线程并放入就绪队列
    // 成功，返回pid；失败，返回-1
    int executeThread(ThreadFunction function, void *parameter, const char *name, int priority);
    // 执行一次调度
    void schedule();

public:
    //  分配一个未被占用的pid
    int allocatePid();
    // 按pid查找线程/进程
    PCB *findProgramByPid(int pid);
    // ListItem转换成PCB
    PCB *ListItem2PCB(ListItem *item);
};

void program_exit();

#endif