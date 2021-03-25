#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "os_type.h"

class InterruptManager
{
private:
    uint32 *IDT;
    uint32 IRQ0_8259A_MASTER;
    uint32 IRQ0_8259A_SLAVE;
    
public:
    InterruptManager();
    void initialize();
    // 1 trap 0 interrupt
    void setInterruptDescriptor(uint32 index, uint32 address, byte DPL);
    void enableTimeInterrupt();
    void disableTimeInterrupt();
    void setTimeInterrupt(void *handler);

private:
    void initialize8259A();
};

#endif