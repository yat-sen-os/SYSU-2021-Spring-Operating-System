#ifndef ASM_UTILS_H
#define ASM_UTILS_H

#include "os_type.h"

extern "C" void asm_halt();
extern "C" void asm_lidt(uint32 start, uint16 limit);
extern "C" void asm_interrupt_empty_handler();
#endif