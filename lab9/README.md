# 再论进程的实现

# 实验概述

# 参考资料

# 实验要求

# fork

## 概述

在unix或类unix的操作系统中，fork函数用于将运行中的进程分成两个（几乎）完全一样的进程，每个进程会从fork的返回点开始执行。

fork函数，顾名思义，其执行逻辑形如叉子。一个简单的叉子由两部分组成，一个长柄和两个短柄。我们不妨把程序的起始点当成是长柄的末端，把fork的点当成是长柄和短柄的连接点，柄就相当于代码执行流。在连接点之前，只有一个长柄，也就是只有一个执行流。但是在fork之后，出现了两个完全相同的短柄，也就是有两个完全相同的代码执行流。这就是fork函数的形象化解释。

# TODO 叉子解释fork示意图

fork是一个系统调用，用于创建一个新进程。被创建的进程称为子进程，调用fork的进程被称为父进程。前面已经提到，子进程是父进程的副本。父子进程共享代码段，但对于数据段、栈段等其他资源，父进程调用的fork函数会将这部分资源完全复制到子进程中。因此，对于这部分资源，父子进程并不共享。

创建新的子进程后，两个进程将从fork的返回点开始执行。这就是fork最精妙的地方，因为我们只调用了fork一次，fork却能够返回两次。同时，在父子进程的fork返回点中，fork返回的结果是不一样的，fork返回值如下。

+ 在父进程中，fork返回新创建子进程的进程ID。
+ 在子进程中，fork返回0。
+ 如果出现错误，fork返回一个[负值](https://baike.baidu.com/item/负值)。

为了进一步理解fork，我们来考虑如下实例代码。注意，下面的代码需要在linux系统下演示。

```cpp
#include <unistd.h>
#include <sys/types.h>
#include <cstdio>
#include <cstdlib>

int main()
{
    printf("call fork\n");

    pid_t pid = fork();
    if (pid < 0)
    {
        printf("fork error\n");
        exit(-1);
    }

    printf("fork return\n");

    if (pid)
    {
        printf("father fork return: %d\n", pid);
    }
    else
    {
        printf("child fork return: %d\n", pid);
    }
}
```

我们编译运行，结果如下。

```shell
nelson@UX430UNR:~/oslab/tmp$ g++ main.cpp
nelson@UX430UNR:~/oslab/tmp$ ./a.out
call fork
fork return
father fork return: 40706
fork return
child fork return: 0
```

可以看到，fork确实是调用一次返回两次。在父进程中，父进程得到的是子进程的pid，40706；在子进程中，子进程得到的是0。

理解了fork的用途后，我们接下来实现fork。

## 四个关键问题

fork的实现可以细化为4个问题的解决。

1. 如何实现父子进程的代码段共享？
2. 如何使得父子进程从相同的返回点开始执行？
3. 除代码段外，进程包含的资源有哪些？
4. 如何实现进程的资源在进程之间的复制？

我们带着上面4个问题来学习fork的实现。

## fork的实现

> 代码在`src/1`下。

我们首先在PCB中加入父进程pid这个属性。

```cpp
struct PCB
{
	...
    int parentPid;            // 父进程pid
};
```

fork是一个系统调用。为此，我们首先在`include/syscall.h`中加入fork系统调用和系统调用处理函数的定义。

```cpp
#ifndef SYSCALL_H
#define SYSCALL_H

...

// 第2个系统调用, fork
int fork();
int syscall_fork();

#endif
```

在`src/kernel/setup.cpp`中设置这个系统调用。

```cpp
...
    
extern "C" void setup_kernel()
{
	...
        
    // 设置2号系统调用
    systemService.setSystemCall(2, (int)syscall_fork);
	
    ...

```

在`src/kernel/syscall.cpp`中实现fork系统调用。

```cpp
int fork() {
    return asm_system_call(2);
}

int syscall_fork() {
    return programManager.fork();
}
```

进入内核态后，fork的实现通过`ProgramManager::fork`来完成。

```cpp
int ProgramManager::fork()
{
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    // 禁止内核线程调用
    PCB *parent = this->running;
    if (!parent->pageDirectoryAddress)
    {
        interruptManager.setInterruptStatus(status);
        return -1;
    }

    // 创建子进程
    int pid = executeProcess("", 0);
    if (pid == -1)
    {
        interruptManager.setInterruptStatus(status);
        return -1;
    }

    // 初始化子进程
    PCB *child = ListItem2PCB(this->allPrograms.back(), tagInAllList);
    bool flag = copyProcess(parent, child);

    if (!flag)
    {
        child->status = ProgramStatus::DEAD;
        interruptManager.setInterruptStatus(status);
        return -1;
    }

    interruptManager.setInterruptStatus(status);
    return pid;
}
```

第6-12行，fork是进程的系统调用，因此我们禁止内核线程调用。因为内核线程并没有设置`PCB::pageDirectoryAddress`，所以该项为0。相反，进程有页目录表，所以该项不为0。因此，我们通过判断`PCB::pageDirectoryAddress`是否为0来判断当前执行的是线程还是进程。

第14-20行，我们调用`ProgramManager::executeProcess`来创建一个子进程。

第23-24行，我们找到刚刚创建的子进程，然后调用`ProgramManager::copyProcess`来复制父进程的资源到子进程中。

接下来，我们来研究fork实现最关键的部分——资源的复制，即函数`ProgramManager::copyProcess`。`copyProcess`有点长，我们分步来看。

我们首先回答第三个问题——除代码段外，进程包含的资源有哪些？在我们的操作系统中，进程包含的资源有0特权级栈，PCB、虚拟地址池、页目录表、页表及其指向的物理页。

首先，我们复制父进程的0特权级栈到子进程中。为此，我们首先需要找到父进程的0特权级栈的地址。

当我们调用fork系统调用后，我们会从用户态进入内核态。进入点就是`asm_system_call`中的`int 0x80`语句。接着，CPU会将进程的0特权级栈的地址送入esp。进程的0特权级栈的地址已经在调度的时候被放入了TSS。

```cpp
void ProgramManager::activateProgramPage(PCB *program)
{
...
        tss.esp0 = (int)program + PAGE_SIZE;
...
}
```

因此，进程的0特权级栈从进程的PCB所在的页的顶部向下扩展。在进程中，`asm_system_call`执行到`int 0x80`后，esp存放的内容变成了进程的PCB所在的页的顶部的地址。然后，CPU将中断发生前的SS，ESP，EFLAGS、CS、EIP送入栈中。接着，我们来到了函数`asm_system_call_handler`

```assembly
asm_system_call_handler:
    push ds
    push es
    push fs
    push gs
    pushad
    
    ...
```

`asm_system_call_handler`将`ds,es,fs,gs`和其他数据寄存器压栈。执行完pushad后，从进程的0特权级栈的栈顶到esp之间的内容恰好对应与`ProgramStartStack`的内容。

因此，我们需要复制的父进程0特权栈的起始地址就是进程的PCB所在的页的顶部减去一个`ProgramStartStack`的大小。这就是`copyProcess`开头的语句。我们实际上就是把在中断的那一刻保存的寄存器的内容复制到子进程的0特权级栈中。

```cpp
    // 复制进程0级栈
    ProcessStartStack *childpss =
        (ProcessStartStack *)((int)child + PAGE_SIZE - sizeof(ProcessStartStack));
    ProcessStartStack *parentpss =
        (ProcessStartStack *)((int)parent + PAGE_SIZE - sizeof(ProcessStartStack));
    memcpy(parentpss, childpss, sizeof(ProcessStartStack));
    // 设置子进程的返回值为0
    childpps->eax = 0;
```

第8行，我们将子进程的eax设置为0，使得在子进程中的fork返回值为0。

为什么我们需要这样做？我们希望在执行fork后，父子进程从相同的点返回。为此，我们需要先将父进程暂停在某个位置，然后令子进程跳转到父进程暂停后的返回点。注意到`ProgramStartProcess`中保存了父进程的`eip`，`eip`的内容也是`asm_system_call_handler`的返回地址。我们后面可以看到，我们会通过`asm_start_process`来启动子进程。此时，`asm_start_process`的最后的`iret`会将上面说到的保存在0特权级栈的`eip`的内容送入到`eip`中。执行完`eip`后，子进程便可以从父进程的返回点处开始执行，即`asm_system_call_handler`的返回地址。然后子进程依次返回到`syscall_fork`，`asm_system_call_handler`，最终从`fork`返回。由于我们后面会复制父进程的3特权级栈到子进程的3特权级栈中，而3特权级栈保存了父进程在执行`int 0x80`后的逐步返回的返回地址。因此，父子进程的逐步返回的地址是相同的，从而实现了在执行fork后，父子进程从相同的点返回。这就是第二个问题的答案。

为了实现父子进程的执行逻辑相同，我们需要实现父子进程的代码共享。我们使用了函数来模拟一个进程，而函数的代码是放在内核中的，进程又划分了3GB\~4GB的空间来实现内核共享，因此进程的代码天然就是共享的。此时，我们便回答了第一个问题。

接着，我们继续初始化子进程的0特权级栈。

```cpp
    // 准备执行asm_switch_thread的栈的内容
    child->stack = (int *)childpss - 7;
    child->stack[0] = 0;
    child->stack[1] = 0;
    child->stack[2] = 0;
    child->stack[3] = 0;
    child->stack[4] = (int)asm_start_process;
    child->stack[5] = 0;             // asm_start_process 返回地址
    child->stack[6] = (int)childpss; // asm_start_process 参数
```

这样做是为了和`asm_switch_thread`的过程对应起来。当子进程被调度执行时，子进程能够从`asm_switch_thread`跳转到`asm_start_proces`处执行。

接着，我们设置子进程的PCB、复制父进程的管理虚拟地址池的bitmap到子进程的管理虚拟地址池的bitmap。

```cpp
    // 设置子进程的PCB
    child->status = ProgramStatus::READY;
    child->parentPid = parent->pid;
    child->priority = parent->priority;
    child->ticks = parent->ticks;
    child->ticksPassedBy = parent->ticksPassedBy;
    strcpy(parent->name, child->name);

    // 复制用户虚拟地址池
    int bitmapLength = parent->userVirtual.resources.length;
    int bitmapBytes = ceil(bitmapLength, 8);
    memcpy(parent->userVirtual.resources.bitmap, child->userVirtual.resources.bitmap, bitmapBytes);
```

然后，我们从内核中分配一页来作为数据复制的中转页。

```cpp
    // 从内核中分配一页作为中转页
    char *buffer = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 1);
    if (!buffer)
    {
        child->status = ProgramStatus::DEAD;
        return false;
    }
```

为什么需要这个中转页呢？因为父子进程使用的是相同的代码。既然代码是相同的，则使用的虚拟地址也是相同的。二者不发生冲突的原因在于分页机制。通过分页机制，我们可以将相同的虚拟地址映射到不同的物理地址中。但是这会带来一个问题。既然分页机制实现了地址隔离，父进程就无法将数据复制到具有相同虚拟地址的子进程中。因此，我们需要借助于内核空间的中转页。我们首先在父进程的虚拟地址空间下将数据复制到中转页中，再切换到子进程的虚拟地址空间中，然后将中转页复制到子进程对应的位置。如此，我们便回答了最后一个问题，即第四个问题。

然后，我们先将父进程的页目录表复制到子进程中。

```cpp
    // 子进程页目录表物理地址
    int childPageDirPaddr = memoryManager.vaddr2paddr(child->pageDirectoryAddress);
    // 父进程页目录表物理地址
    int parentPageDirPaddr = memoryManager.vaddr2paddr(parent->pageDirectoryAddress);
    // 子进程页目录表指针(虚拟地址)
    int *childPageDir = (int *)child->pageDirectoryAddress;
    // 父进程页目录表指针(虚拟地址)
    int *parentPageDir = (int *)parent->pageDirectoryAddress;

    // 子进程页目录表初始化
    memset((void *)child->pageDirectoryAddress, 0, 768 * 4);

    // 复制页目录表
    for (int i = 0; i < 768; ++i)
    {
        // 无对应页表
        if (!(parentPageDir[i] & 0x1))
        {
            continue;
        }

        // 从用户物理地址池中分配一页，作为子进程的页目录项指向的页表
        int paddr = memoryManager.allocatePhysicalPages(AddressPoolType::USER, 1);
        if (!paddr)
        {
            child->status = ProgramStatus::DEAD;
            return false;
        }
        // 页目录项
        int pde = parentPageDir[i];
        // 构造页表的起始虚拟地址
        int *pageTableVaddr = (int *)(0xffc00000 + (i << 12));

        asm_update_cr3(childPageDirPaddr); // 进入子进程虚拟地址空间

        childPageDir[i] = (pde & 0x00000fff) | paddr;
        memset(pageTableVaddr, 0, PAGE_SIZE);

        asm_update_cr3(parentPageDirPaddr); // 回到父进程虚拟地址空间
    }
```

第17-20行，我们首先检查目录项是否有对应的页表。

第23-28行，页目录项有指向的页表的前提下。我们从用户物理空间中分配一个物理页作为页目录项指向的页表。

第34行，我们切换到子进程的虚拟地址空间。

第36-37行，我们设置子进程页目录表的页目录项，然后初始化页目录项指向的页表。特别注意，切换到子进程的虚拟空间后，`pageTableVaddr`在解析到物理地址时使用的是子进程的页目录表。

第39行，我们切换到父进程的虚拟地址空间。

处理完子进程的页目录表后，我们复制页表和物理页的数据。

```cpp
    // 复制页表和物理页
    for (int i = 0; i < 768; ++i)
    {
        // 无对应页表
        if (!(parentPageDir[i] & 0x1))
        {
            continue;
        }

        // 计算页表的虚拟地址
        int *pageTableVaddr = (int *)(0xffc00000 + (i << 12));

        // 复制物理页
        for (int j = 0; j < 1024; ++j)
        {
            // 无对应物理页
            if (!(pageTableVaddr[j] & 0x1))
            {
                continue;
            }

            // 从用户物理地址池中分配一页，作为子进程的页表项指向的物理页
            int paddr = memoryManager.allocatePhysicalPages(AddressPoolType::USER, 1);
            if (!paddr)
            {
                child->status = ProgramStatus::DEAD;
                return false;
            }

            // 构造物理页的起始虚拟地址
            void *pageVaddr = (void *)((i << 22) + (j << 12));
            // 页表项
            int pte = pageTableVaddr[j];
            // 复制出父进程物理页的内容到中转页
            memcpy(pageVaddr, buffer, PAGE_SIZE);

            asm_update_cr3(childPageDirPaddr); // 进入子进程虚拟地址空间

            pageTableVaddr[j] = (pte & 0x00000fff) | paddr;
            // 从中转页中复制到子进程的物理页
            memcpy(buffer, pageVaddr, PAGE_SIZE);

            asm_update_cr3(parentPageDirPaddr); // 回到父进程虚拟地址空间
        }
    }
```

第14-45行，在页目录项指向的页表存在的前提下，我们复制页表项和物理页。

第17-20行，我们检查页表项指向的页表是否存在。

第23-28行，在页表项指向的页表存在前提下，我们从用户物理地址池中分配一页，作为子进程的页表项指向的物理页。

第35行，我们复制父进程的物理页的数据到中转页中。

第37行，我们切换到子进程的虚拟地址空间。

第41行，我们设置页表项，然后从中转页中复制数据到子进程的物理页中。

第43行，我们切换到父进程的虚拟地址空间。

最后，我们归还中转页，返回子进程的pid。

```cpp
    // 归还从内核分配的中转页
    memoryManager.releasePages(AddressPoolType::KERNEL, (int)buffer, 1);
    return true;
```

代码中的`memcpy`函数的实现放置在`src/utils/stdlib.cpp`中。

如此，我们便实现了fork。

接下来我们来测试fork，我们对`src/kernel/setup.cpp`做如下修改。

```cpp
void first_process()
{
    int pid = fork();

    if (pid == -1)
    {
        printf("can not fork\n");
    }
    else
    {
        if (pid)
        {
            printf("I am father, fork reutrn: %d\n", pid);
        }
        else
        {
            printf("I am child, fork return: %d, my pid: %d\n", pid, 
                   programManager.running->pid);
        }
    }

    asm_halt();
}

void first_thread(void *arg)
{

    printf("start process\n");
    programManager.executeProcess((const char *)first_process, 1);
    asm_halt();
}
```

编译运行，结果如下。

<img src="/home/nelson/oslab/lab9/gallery/fork的实现.png" alt="fork的实现" style="zoom:80%;" />

从结果可以看到，我们已经成功实现了fork。

# exit

> 代码放置在`src/2`下。

有时候我们在进程或线程中希望主动结束运行，此时，我们就需要用到系统调用exit。exit用于进程和线程的主动结束运行，如下所示，代码放置在`include/syscall.h`。

```cpp
// 第3个系统调用, exit
void exit(int ret);
void syscall_exit(int ret);
```

其中，`ret`参数表示返回值。

在进程或线程调用exit后，我们会释放其占用的所有资源，只保留PCB。此时线程或进程的状态被标记为`DEAD`。进程或线程的PCB由专门的线程或进程来回收。在PCB被回收之前的`DEAD`线程或进程也被称为僵尸线程或僵尸进程。

注意到我们调用exit时提供了返回值，返回值的处理是在线程或进程的PCB回收时进行的。但是，线程或进程除PCB以外的资源都被释放了，那么返回值放置在哪里呢？我们很容易地想到，返回值是放置在PCB中的。因此，我们为PCB加入存放返回值的属性。

```cpp
struct PCB
{
	...
    int retValue;             // 返回值
};
```

我们现在来实现上面两个函数。

```cpp
void exit(int ret) {
    asm_system_call(3, ret);
}

void syscall_exit(int ret) {
    programManager.exit(ret);
}
```

exit的实现实际上是通过`ProgramManager::exit`来完成的，总的来看，exit的实现主要分为三步。

1. 标记PCB状态为`DEAD`并放入返回值。
2. 如果PCB标识的是进程，则释放进程所占用的物理页、页表、页目录表和虚拟地址池bitmap的空间。否则不做处理。
3. 立即执行线程/进程调度。

如下所示。

```cpp
void ProgramManager::exit(int ret)
{
    // 关中断
    interruptManager.disableInterrupt();
    
    // 第一步，标记PCB状态为`DEAD`并放入返回值。
    PCB *program = this->running;
    program->retValue = ret;
    program->status = ProgramStatus::DEAD;

    int *pageDir, *page;
    int paddr;

    // 第二步，如果PCB标识的是进程，则释放进程所占用的物理页、页表、页目录表和虚拟地址池bitmap的空间。
    if (program->pageDirectoryAddress)
    {
        pageDir = (int *)program->pageDirectoryAddress;
        for (int i = 0; i < 768; ++i)
        {
            if (!(pageDir[i] & 0x1))
            {
                continue;
            }

            page = (int *)(0xffc00000 + (i << 12));

            for (int j = 0; j < 1024; ++j)
            {
                if(!(page[j] & 0x1)) {
                    continue;
                }

                paddr = memoryManager.vaddr2paddr((i << 22) + (j << 12));
                memoryManager.releasePhysicalPages(AddressPoolType::USER, paddr, 1);
            }

            paddr = memoryManager.vaddr2paddr((int)page);
            memoryManager.releasePhysicalPages(AddressPoolType::USER, paddr, 1);
        }

        memoryManager.releasePages(AddressPoolType::KERNEL, (int)pageDir, 1);
        
        int bitmapBytes = ceil(program->userVirtual.resources.length, 8);
        int bitmapPages = ceil(bitmapBytes, PAGE_SIZE);

        memoryManager.releasePages(AddressPoolType::KERNEL,
                                   (int)program->userVirtual.resources.bitmap, 
                                   bitmapPages);

    }

    // 第三步，立即执行线程/进程调度。
    schedule();
}
```

代码比较简单，实际上可以看成是`copyProcess`的部分逆过程，这里便不再赘述。

上面说到的是主动调用exit的情况。我们看到，线程在退出的时候可以自动调用`program_exit`返回，而我们一直在强调进程还没有实现返回机制，所以需要在进程的末尾加上`asm_halt`来阻止进程返回。但是，我们已经实现了进程退出函数exit。那么进程有没有办法像线程一样在函数结束后主动调用exit来结束运行呢？答案是肯定的，我们只要在进程的3特权级栈中放入exit的地址和参数即可，当执行进程的函数退出后就会主动跳转到exit。

为此，我们需要修改`load_process`，如下所示。

```cpp
void load_process(const char *filename)
{
	...

    interruptStack->esp = memoryManager.allocatePages(AddressPoolType::USER, 1);
    if (interruptStack->esp == 0)
    {
        printf("can not build process!\n");
        process->status = ProgramStatus::DEAD;
        asm_halt();
    }
    interruptStack->esp += PAGE_SIZE;
    
    // 设置进程返回地址
    int *userStack = (int *)interruptStack->esp;
    userStack -= 3;
    userStack[0] = (int)exit;
    userStack[1] = 0;
    userStack[2] = 0;

    interruptStack->esp = (int)userStack;

	...
}

```

第17行，我们在进程的3特权级栈中的栈顶处`userStack[0]`放入exit的地址，然后CPU会认为`userStack[1]`是exit的返回地址，`userStack[2]`是exit的参数。

此时，我们修改`src/kernel/setup.cpp`来测试。

```cpp
void first_process()
{
    int pid = fork();

    if (pid == -1)
    {
        printf("can not fork\n");
        asm_halt();
    }
    else
    {
        if (pid)
        {
            printf("I am father\n");
            asm_halt();
        }
        else
        {
            printf("I am child, exit\n");
        }
    }
}

void second_thread(void *arg) {
    printf("thread exit\n");
    exit(0);
}

void first_thread(void *arg)
{

    printf("start process\n");
    programManager.executeProcess((const char *)first_process, 1);
    programManager.executeThread(second_thread, nullptr, "second", 1);
    asm_halt();
}
```

编译运行，输出如下结果。

<img src="/home/nelson/oslab/lab9/gallery/exit的实现.png" alt="exit的实现" style="zoom:80%;" />

可以看到我们已经成功实现了exit。

# wait



# TODO

asm_syscall_handler修改



# 习题

孤儿进程的回收

僵尸进程的回收

子进程的返回点

跟踪exit