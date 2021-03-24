#include "os_type.h"
#include "os_modules.h"
#include "asm_utils.h"

extern "C" void setup_kernel()
{
    // 中断处理部件
    interruptManager.initialize();
    asm_halt();
}