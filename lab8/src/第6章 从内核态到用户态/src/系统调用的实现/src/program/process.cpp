#include "program.h"
#include "interrupt.h"
#include "memory.h"
#include "os_constant.h"
#include "stdlib.h"
#include "process.h"

extern InterruptManager interruptManager;
extern MemoryManager memoryManager;
//extern TSS tss;
/*
void load_process(const char *filename)
{
    PCB *process = programManager.running;
    int *interruptStack = (int *)((int)process + PAGE_SIZE - sizeof(ProcessStartStack));
 
    interruptStack->edi = 0;
    interruptStack->esi = 0;
    interruptStack->ebp = 0;
    interruptStack->esp_dummy = 0;
    interruptStack->ebx = 0;
    interruptStack->edx = 0;
    interruptStack->ecx = 0;
    interruptStack->eax = 0;

    interruptStack->gs = 0;
    interruptStack->fs = 0x3b;
    interruptStack->es = 0x3b;
    interruptStack->ss = 0x3b;
    interruptStack->ds = 0x3b;

    interruptStack->eip = (int)filename;
    interruptStack->cs = 0x33;                                // 用户模式平坦模式
    interruptStack->eflags = (3 << 12) | (1 << 9) | (1 << 1); // IOPL, IF, MBS
    interruptStack->esp = (dword)specifyPaddrForVaddr(AddressPoolType::USER, USER_STACK_VADDR) + PAGE_SIZE;
    interruptStack->esp -= 3 * sizeof(dword);
    // 设置返回process地址
    ((dword *)(interruptStack->esp))[0] = (dword)exit;
    // 1 被认为是返回地址
    ((dword *)(interruptStack->esp))[2] = 1;

    sys_start_process((dword)interruptStack);
}

int ProgramManager::executeProcess(const char *filename, int priority)
{
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    int pid = executeThread((ThreadFunction)load_process, nullptr, filename, priority);
    if (pid == -1)
    {
        return -1;
    }

    PCB *process = ListItem2PCB(allPrograms.back());
    printf("%d %d\n", pid, process->pid);

    process->pageDirectoryAddress = createProcessPageDirectory();
    if (process->pageDirectoryAddress)
    {
        process->status = ThreadStatus::DEAD;
        return -1;
    }

    bool res = createUserVirtualPool(process);
    if (!res)
    {
        process->status = ThreadStatus::DEAD;
        return -1;
    }

    interruptManager.setInterruptStatus(status);
}

int ProgramManager::createProcessPageDirectory()
{
    int vaddr = memoryManager.allocatePages(AddressPoolType::KERNEL, 1);
    if (!vaddr)
    {
        //printf("can not create page from kernel\n");
        return 0;
    }

    memset((char *)vaddr, 0, PAGE_SIZE);

    // 复制内核目录项到虚拟地址的高1GB
    int *src = (int *)(0xfffff000 + 0x300 * 4);
    int *dst = (int *)(vaddr + 0x300 * 4);
    for (int i = 0; i < 256; ++i)
    {
        dst[i] = src[i];
    }

    // 最后一项设置为用户进程页目录表物理地址
    ((int *)vaddr)[1023] = memoryManager.vaddr2paddr((dword)vaddr) | 0x7;
    return vaddr;
}

bool ProgramManager::createUserVirtualPool(PCB *process)
{
    int sourcesCount = (0xc0000000 - USER_VADDR_START) / PAGE_SIZE;
    int bitmapLength = ceil(sourcesCount, 8);

    // 计算位图所占的页数
    int pagesCount = ceil(bitmapLength, PAGE_SIZE);

    int start = allocatePages(AddressPoolType::KERNEL, pagesCount);
    if (!start)
    {
        return false;
    }

    memset((char *)start, 0, PAGE_SIZE * pagesCount);
    (process->userVirtual).initialize((char *)start, bitmapLength, USER_VADDR_START);

    return true;
}

void ProgramManager::updateESP0(PCB *process) {
    // 0特权级栈在PCB中
    tss.esp0 = (int*)((int)process + PAGE_SIZE);
}

*/