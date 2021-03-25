; 不加会出现 ld: warning: cannot find entry symbol _start;
global _start

extern setup_kernel

_start:
    jmp setup_kernel