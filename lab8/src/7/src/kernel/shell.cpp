#include "shell.h"
#include "asm_utils.h"
#include "syscall.h"
#include "stdio.h"

Shell::Shell()
{
    initialize();
}

void Shell::initialize()
{
}

void Shell::run()
{
    move_cursor(0, 0);
    for(int i = 0; i < 25; ++i ) {
        for(int j = 0; j < 80; ++j ) {
            printf(" ");
        }
    }
    move_cursor(0, 0);

    printLogo();

    move_cursor(7, 26);
    printf("Welcome to YatSenOS Project!\n");
    printf("             https://github.com/YatSenOS/YatSenOS-Tutorial-Volume-1\n\n");
    printf("  YatSenOS is an OS course project.\n\n"
           "  Proposed and led by\n"
           "           Prof. PengFei Chen.\n\n"
           "  Developed by\n"
           "           Prof. PengFei Chen,\n"
           "           HPC and AI students in grade 2019,\n"
           "           HongYang Chen,\n"
           "           WenXin Xie,\n"
           "           YuZe Fu,\n"
           "           Nelson Cheung.\n\n"
           );
    

    asm_halt();
}

void Shell::printLogo()
{
    move_cursor(0, 17);
    printf("__   __    _   ____              ___  ____ \n");
    move_cursor(1, 17);
    printf("\\ \\ / /_ _| |_/ ___|  ___ _ __  / _ \\/ ___| \n");
    move_cursor(2, 17);
    printf(" \\ V / _` | __\\___ \\ / _ \\ '_ \\| | | \\___ \\ \n");
    move_cursor(3, 17);
    printf("  | | (_| | |_ ___) |  __/ | | | |_| |___) |\n");
    move_cursor(4, 17);
    printf("  |_|\\__,_|\\__|____/ \\___|_| |_|\\___/|____/ \n");
}