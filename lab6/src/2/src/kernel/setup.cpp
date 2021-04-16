#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"
#include "sync.h"

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;

int shared_variable;
SpinLock aLock;

void second_thread(void *arg) {
    int delay = 0xffffff;

    aLock.lock();

    printf("pid: %d, read: %d\n", programManager.running->pid, shared_variable);
    shared_variable += 10;

    printf("pid: %d, do some time-consuming work\n", programManager.running->pid);
    delay = 0xfffffff;
    while(delay) --delay;
    printf("pid: %d, done\n", programManager.running->pid);

    printf("pid: %d, read: %d\n", programManager.running->pid, shared_variable);

    aLock.unlock();

}

void third_thread(void *arg) {

    printf("pid: %d, start to run\n", programManager.running->pid);

    aLock.lock();

    printf("pid: %d, read: %d\n", programManager.running->pid, shared_variable);
    shared_variable -= 10;
    printf("pid: %d, read: %d\n", programManager.running->pid, shared_variable);
    
    aLock.unlock();
}
void first_thread(void *arg)
{
    // 第1个线程不可以返回
    printf("pid %d name \"%s\": Hello World!\n", programManager.running->pid, programManager.running->name);
    aLock.initialize();

    if (!programManager.running->pid)
    {
        programManager.executeThread(second_thread, nullptr, "second thread", 1);
        programManager.executeThread(third_thread, nullptr, "third thread", 1);
    }

    while(1) {

    }
    //asm_halt();
}

extern "C" void setup_kernel()
{

    // 中断管理器
    interruptManager.initialize();
    interruptManager.enableTimeInterrupt();
    interruptManager.setTimeInterrupt((void *)asm_time_interrupt_handler);

    // 输出管理器
    stdio.initialize();

    // 进程/线程管理器
    programManager.initialize();

    // 创建第一个线程
    int pid = programManager.executeThread(first_thread, nullptr, "first thread", 1);
    if (pid == -1)
    {
        printf("can not execute thread\n");
        asm_halt();
    }

    ListItem *item = programManager.readyPrograms.front();
    PCB *firstThread = ListItem2PCB(item, tagInGeneralList);
    firstThread->status = RUNNING;
    programManager.readyPrograms.pop_front();
    programManager.running = firstThread;
    asm_switch_thread(0, firstThread);

    asm_halt();
}
