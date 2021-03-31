#include <iostream>

template<typename T>
void swap(T &x, T &y) {
    T z = x;
    x = y;
    y = z;
}


void itos(char *numStr, unsigned int num, unsigned int mod) {
    numStr[0] = '\0';

    if (mod < 2 || mod > 26 || num < 0) {
        return;
    }

    unsigned int length, temp;

    length = 0;
    while(num) {
        temp = num % mod;
        num /= mod;
        numStr[length] = temp > 9 ? temp - 10 + 'A' : temp + '0';
        ++length;
    }

    if(!length) {
        numStr[0] = '0';
        ++length;
    }

    for(int i = 0, j = length - 1; i < j; ++i, --j) {
        swap(numStr[i], numStr[j]);
    }
}

int main() {
    char number[40];

    itos(number, 23012, 10);
    itos(number, 12312, 10);

    std::cout << number << std::endl;

}