#ifndef THREAD_H
#define THREAD_H

#include "os_constant.h"
#include "list.h"
#include "address_pool.h"

typedef void (*ThreadFunction)(void *);

enum ThreadStatus
{
    CREATE,
    RUNNING,
    READY,
    BLOCKED,
    DEAD
};

struct PCB
{
    int *stack;                      // 栈指针，用于调度时保存esp
    char name[MAX_PROGRAM_NAME + 1]; // 线程名
    enum ThreadStatus status;        // 线程的状态
    int priority;                    // 线程优先级
    int pid;                         // 线程pid
    int ticks;                       // 线程时间片总时间
    int ticksPassedBy;               // 线程已执行时间
    ListItem tagInGeneralList;       // 线程队列标识
    ListItem tagInAllList;           // 线程队列标识
    int pageDirectoryAddress;        // 页目录表地址
    AddressPool userVirtual;         // 用户程序虚拟地址池
};

#endif