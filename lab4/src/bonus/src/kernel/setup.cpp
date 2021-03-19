#include "os_type.h"
#include "os_modules.h"
#include "asm_utils.h"
#include "stdio.h"

extern "C" void setup_kernel()
{
    // 中断处理部件
    interruptManager.initialize();
    // 屏幕IO处理部件
    stdio.initialize();
    interruptManager.enableTimeInterrupt();
    interruptManager.setTimeInterrupt((void *)asm_time_interrupt_handler);
    asm_enable_interrupt();
    //uint a = 1 / 0;
    asm_halt();
}