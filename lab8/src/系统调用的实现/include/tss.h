#ifndef TSS_H
#define TSS_H

extern "C" int sys_add_gd(int low, int high);
extern "C" void sys_init_tss(int selector);

struct TSS
{
public:
    int backlink;
    int esp0;
    int ss0;
    int esp1;
    int ss1;
    int esp2;
    int ss2;
    int cr3;
    int eip;
    int eflags;
    int eax;
    int ecx;
    int edx;
    int ebx;
    int esp;
    int ebp;
    int esi;
    int edi;
    int es;
    int cs;
    int ss;
    int ds;
    int fs;
    int gs;
    int ldt;
    int trace;
    int ioMap;
};
/*
public:
    void updateEsp0(PCB *thread) {
        esp0 = (int*)((int)thread + PAGE_SIZE);
    }

    void initialize() {
        int size = sizeof(Tss);
        memset((byte *)this, 0, size);
        ss0 = 0x10; // 系统堆栈段选择子
        int low, high, temp;
        low = 0;
        temp = (size - 1) & 0xff;
        low = low | temp;
        temp = (int)this << 16;
        low = low | temp;

        high = 0;
        temp = (int)this & 0xff000000;
        high = high | temp;
        temp = (int)this & 0x00ff0000;
        temp = temp >> 16;
        high = high | temp;
        temp = (size - 1) & 0xff00;
        temp = temp << 16;
        high = high | temp;
        high = high | 0x00008900;

        temp = sys_add_gd(low, high);
        temp = temp << 3;
        sys_init_tss(temp);
        sys_add_gd(USER_CODE_LOW, USER_CODE_HIGH);
        sys_add_gd(USER_DATA_LOW, USER_DATA_HIGH);

        ioMap = (int)this + size;
    }
};
*/
#endif