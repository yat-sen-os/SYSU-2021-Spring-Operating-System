#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "os_type.h"

class InterruptManager
{
private:
    uint32 *IDT;              // IDT起始地址
    uint32 IRQ0_8259A_MASTER; // 主片中断起始向量号
    uint32 IRQ0_8259A_SLAVE;  // 从片中断起始向量号

public:
    InterruptManager();
    void initialize();
    // 设置中断描述符
    // index   第index个描述符，index=0, 1, ..., 255
    // address 中断处理程序的起始地址
    // DPL     中断描述符的特权级
    void setInterruptDescriptor(uint32 index, uint32 address, byte DPL);
    // 开启时钟中断
    void enableTimeInterrupt();
    // 禁止时钟中断
    void disableTimeInterrupt();
    // 设置时钟中断处理函数
    void setTimeInterrupt(void *handler);

    // 开中断
    void enableInterrupt();
    // 关中断
    void disableInterrupt();
    // 获取中断状态
    // 返回true，中断开启；返回false，中断关闭
    bool getInterruptStatus();
    // 设置中断状态
    // status=true，开中断；status=false，关中断
    void setInterruptStatus(bool status);

private:
    // 初始化8259A芯片
    void initialize8259A();
};

#endif