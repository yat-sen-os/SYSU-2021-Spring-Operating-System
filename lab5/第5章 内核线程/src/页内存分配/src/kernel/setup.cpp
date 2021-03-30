#include "os_type.h"
#include "os_modules.h"
#include "asm_utils.h"
#include "stdio.h"
#include "stdlib.h"
#include "bitmap.h"
#include "os_constant.h"

void print_bitmap(const BitMap &bitmap);

extern "C" void setup_kernel()
{
    // 中断管理器
    interruptManager.initialize();
    interruptManager.enableTimeInterrupt();
    interruptManager.setTimeInterrupt((void *)asm_time_interrupt_handler);

    // 输出管理器
    stdio.initialize();

    // 内存管理器
    memoryManager.openPageMechanism();
    memoryManager.initialize(32 * 1024 * 1024);

    //asm_enable_interrupt();

    asm_halt();
}

void print_bitmap(const BitMap &bitmap)
{
    printf("bitmap status: ");
    for (int i = 0; i < bitmap.size(); ++i)
    {
        printf("%d", bitmap.get(i));
    }
    printf("\n");
}
