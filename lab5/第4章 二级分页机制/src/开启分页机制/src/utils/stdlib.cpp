int itos(char *numStr, int num, int mod)
{
    if (mod < 2 || mod > 26 || num < 0)
    {
        return -1;
    }

    int length, temp;

    length = 0;
    while (num)
    {
        temp = num % mod;
        num /= mod;
        numStr[length] = temp > 9 ? temp - 10 + 'A' : temp + '0';
        ++length;
    }

    if (!length)
    {
        numStr[0] = '0';
        ++length;
    }

    return length;
}

void memset(void *memory, char value, int length)
{
    for (int i = 0; i < length; ++i)
    {
        ((char *)memory)[i] = value;
    }
}