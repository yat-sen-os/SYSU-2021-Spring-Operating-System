#include "program.h"
#include "memory.h"
#include "stdlib.h"
#include "interrupt.h"
#include "asm_utils.h"
#include "stdio.h"

extern MemoryManager memoryManager;
extern InterruptManager interruptManager;
extern ProgramManager programManager;

ProgramManager::ProgramManager()
{
    initialize();
}

void ProgramManager::initialize()
{
    allPrograms.initialize();
    readyPrograms.initialize();
    running = nullptr;
}

int ProgramManager::executeThread(ThreadFunction function, void *parameter, const char *name, int priority)
{
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    PCB *thread = (PCB *)memoryManager.allocatePages(AddressPoolType::KERNEL, 1);
    if (!thread)
        return -1;

    memset((char *)thread, 0, PAGE_SIZE);

    for (int i = 0; i < MAX_PROGRAM_NAME && name[i]; ++i)
    {
        thread->name[i] = name[i];
    }

    thread->status = ThreadStatus::READY;
    thread->priority = priority;
    thread->ticks = priority * 10;
    thread->ticksPassedBy = 0;

    thread->pid = allocatePid();
    if (thread->pid == -1)
    {
        memoryManager.releasePages(AddressPoolType::KERNEL, (int)thread, 1);
        return -1;
    }

    // 线程栈
    thread->stack = (int *)((int)thread + PAGE_SIZE);
    thread->stack -= 7;
    thread->stack[0] = 0;
    thread->stack[1] = 0;
    thread->stack[2] = 0;
    thread->stack[3] = 0;
    thread->stack[4] = (int)function;
    thread->stack[5] = (int)program_exit;
    thread->stack[6] = (int)parameter;

    allPrograms.push_back(&(thread->tagInAllList));
    readyPrograms.push_back(&(thread->tagInGeneralList));

    interruptManager.setInterruptStatus(status);

    return thread->pid;
}

int ProgramManager::allocatePid()
{
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    // 0号线程
    if (allPrograms.empty())
        return 0;

    int pid = -1;
    PCB *program;

    for (int i = 0; i < MAX_PROGRAM_AMOUNT; ++i)
    {
        program = findProgramByPid(i);
        if (!program)
        {
            pid = i;
            break;
        }
    }

    interruptManager.setInterruptStatus(status);

    return pid;
}

PCB *ProgramManager::findProgramByPid(int pid)
{
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    ListItem *item = allPrograms.head.next;
    PCB *program, *ans;

    ans = nullptr;
    while (item)
    {

        program = (PCB *)(((dword)item) & 0xfffff000);

        if (program->pid == pid)
        {
            ans = program;
            break;
        }
        item = item->next;
    }

    interruptManager.setInterruptStatus(status);

    return ans;
}

void ProgramManager::schedule()
{
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    if (readyPrograms.size() == 0)
    {
        interruptManager.setInterruptStatus(status);
        return;
    }

    if (running->status == ThreadStatus::RUNNING)
    {
        running->status = ThreadStatus::READY;
        running->ticks = running->priority * 10;
        readyPrograms.push_back(&(running->tagInGeneralList));
    }

    ListItem *item = readyPrograms.front();
    PCB *next = ListItem2PCB(item);
    PCB *cur = running;
    next->status = ThreadStatus::RUNNING;
    running = next;
    readyPrograms.pop_front();

    //printf("schedule: %x, %x\n", cur, next);

    asm_switch_thread(cur->stack, next->stack);

    interruptManager.setInterruptStatus(status);
}

PCB *ProgramManager::ListItem2PCB(ListItem *item)
{
    return (PCB *)(((int)item) & 0xfffff000);
}

void program_exit()
{
    PCB *thread = programManager.running;
    thread->status = ThreadStatus::DEAD;

    if (thread->pid)
    {
        programManager.schedule();
    }
    else
    {
        interruptManager.disableInterrupt();
        printf("halt\n");
        asm_halt();
    }
}