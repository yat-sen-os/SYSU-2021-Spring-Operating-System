#ifndef PROGRAM_H
#define PROGRAM_H

#include "list.h"
#include "thread.h"

#define ListItem2PCB(ADDRESS, LIST_ITEM) ((PCB *)((int)(ADDRESS) - (int)&((PCB *)0)->LIST_ITEM))

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


    // 按pid查找线程/进程
    PCB *findProgramByPid(int pid);
    // 分配一个PCB
    PCB *allocatePCB();
    // 归还一个PCB
    void releasePCB(PCB *program);
};

void program_exit();

#endif