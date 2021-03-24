#include "interrupt.h"
#include "os_type.h"
#include "os_constant.h"
#include "asm_utils.h"
#include "stdio.h"

extern STDIO stdio;
struct Snake
{
    int length;
    Pair<int> direction;
    Pair<int> body[20];
    byte str[20];
};

Snake snake;

InterruptManager::InterruptManager()
{
    initialize();
}

void InterruptManager::initialize()
{
    snake.length = 6;
    snake.direction.first = 0;
    snake.direction.second = 1;

    for (int i = 0; i < snake.length; ++i)
    {
        snake.body[i].first = 0;
        snake.body[i].second = i;
    }

    // 初始化IDT
    IDT = (uint32 *)IDT_START_ADDRESS;
    asm_lidt(IDT_START_ADDRESS, 256 * 8 - 1);

    for (uint i = 0; i < 256; ++i)
    {
        setInterruptDescriptor(i, (uint32)asm_interrupt_empty_handler, 0);
    }

    // 初始化8259A芯片
    initialize8259A();
}

void InterruptManager::setInterruptDescriptor(uint32 index, uint32 address, byte DPL)
{
    IDT[index * 2] = (CODE_SELECTOR << 16) | (address & 0xffff);
    IDT[index * 2 + 1] = (address & 0xffff0000) | (0x1 << 15) | (DPL << 13) | (0xe << 8) ;
}

void InterruptManager::initialize8259A()
{
    // ICW 1
    asm_out_port(0x20, 0x11);
    asm_out_port(0xa0, 0x11);
    // ICW 2
    IRQ0_8259A_MASTER = 0x20;
    IRQ0_8259A_SLAVE = 0x28;
    asm_out_port(0x21, IRQ0_8259A_MASTER);
    asm_out_port(0xa1, IRQ0_8259A_SLAVE);
    // ICW 3
    asm_out_port(0x21, 4);
    asm_out_port(0xa1, 2);
    // ICW 4
    asm_out_port(0x21, 1);
    asm_out_port(0xa1, 1);

    // OCW 1 屏蔽从片所有中断，屏蔽主片所有中断，但开启从片中断
    asm_out_port(0x21, 0xfb);
    asm_out_port(0xa1, 0xff);
}

void InterruptManager::enableTimeInterrupt()
{
    uint8 value;

    /*
    // 设置和时钟中断相关的硬件
    asm_out_port(0x70, 0x8b);
    asm_out_port(0x71, 0x12);
    // 读寄存器C
    asm_out_port(0x70, 0x0c);
    asm_in_port(0x71, &value);
    */

    // 读入主片OCW
    asm_in_port(0x21, &value);
    // 开启主片时钟中断，置0开启
    value = value & 0xfe;
    asm_out_port(0x21, value);
}

void InterruptManager::disableTimeInterrupt()
{
    uint8 value;
    asm_in_port(0x21, &value);
    // 关闭时钟中断，置1关闭
    value = value | 0x01;
    asm_out_port(0x21, value);
}

void InterruptManager::setTimeInterrupt(void *handler)
{
    setInterruptDescriptor(IRQ0_8259A_MASTER, (uint32)handler, 0);
}

void c_time_interrupt_handler()
{
    Pair<int> head = snake.body[snake.length - 1];
    head.first += snake.direction.first;
    head.second += snake.direction.second;
    if (head.first >= 25 || head.first < 0)
    {
        if (head.first < 0)
        {
            head.first = 0;
        }
        else
        {
            head.first = 24;
        }

        snake.direction.first = 0;
        if (head.second)
        {
            head.second = 78;
            snake.direction.second = -1;
        }
        else
        {
            head.second = 1;
            snake.direction.second = 1;
        }
    }
    else if (head.second >= 80 || head.second < 0)
    {
        if (head.second < 0)
        {
            head.second = 0;
        }
        else
        {
            head.second = 79;
        }

        snake.direction.second = 0;
        if (head.first)
        {
            head.first = 23;
            snake.direction.first = -1;
        }
        else
        {
            head.first = 1;
            snake.direction.first = 1;
        }
    }

    stdio.print(snake.body[0].first, snake.body[0].second, ' ', 0x07);

    for (int i = 0; i < snake.length - 1; ++i)
    {
        snake.body[i] = snake.body[i + 1];
    }

    snake.body[snake.length - 1] = head;

    for (int i = 0; i < snake.length; ++i)
    {
        stdio.print(snake.body[i].first, snake.body[i].second, '*', 0x03);
    }
}