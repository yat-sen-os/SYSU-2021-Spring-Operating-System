#ifndef MEMORY_H
#define MEMORY_H

#include "address_pool.h"

enum AddressPoolType
{
    USER,
    KERNEL
};

class MemoryManager
{
public:
    // 内核物理地址池
    AddressPool kernelPhysical;
    // 用户物理地址池
    AddressPool userPhysical;
    // 内核虚拟地址池
    AddressPool kernelVirtual;

public:
    // 开启分页机制
    void openPageMechanism();

    // 初始化地址池
    void initialize(const int TOTAL_MEMORY);

    // 分配count个连续的页地址空间并返回起始地址，type决定了是从用户空间还是从内核空间分配页内存。
    // 成功，返回起始地址；失败，返回0
    int allocatePages(enum AddressPoolType type, const int count);

public:
    // 从type类型的虚拟地址池中分配count个连续的页
    // 成功，返回起始地址；失败，返回0
    int allocateVirtualPages(enum AddressPoolType type, const int count);

    // 从type类型的物理地址池中分配count个连续的页
    // 成功，返回起始地址；失败，返回0
    int allocatePhysicalPages(enum AddressPoolType type, const int count);

    // 建立从虚拟地址到物理地址的映射关系
    // 成功，返回true；失败，返回false
    bool connectPhysicalVritualPage(const int virtualAddress, const int physicalPageAddress);

    // 构造virtualAddress对应的页目录项的虚拟地址
    int toPDE(const int virtualAddress);

    // 构造virtualAddress对应的页表项的虚拟地址
    int toPTE(const int virtualAddress);

    // 释放虚拟页
    void releaseVirtualPages(enum AddressPoolType type, const int vaddr, const int count);

    // 释放物理页
    void releasePhysicalPages(enum AddressPoolType type, const int paddr, const int count);

    // 释放页内存
    void releasePages(enum AddressPoolType type, const int virtualAddress, const int count);

    // 虚拟地址向物理地址转换
    int vaddr2paddr(int vaddr);
    
private:
};
#endif