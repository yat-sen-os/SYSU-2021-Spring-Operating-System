#ifndef STDLIB_H
#define STDLIB_H

#include "os_type.h"

int itos(char *numStr, uint num, uint mod);
void memset(void *memory, char value, int length);
int ceil(const int dividend, const int divisor);

#endif