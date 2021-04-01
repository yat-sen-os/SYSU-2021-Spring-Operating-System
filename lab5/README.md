# 实验要求

+ gdb查看可变参数的例子。
+ printf %d %x
+ 线程调度算法。

# 参考资料

+ [可变参数函数详解](https://www.cnblogs.com/clover-toeic/p/3736748.html)
+ 《操作系统真象还原》

# printf的实现

在Lab4中，我们已经将向屏幕输出字符的函数封装在了类`STDIO`中。相比于使用gdb，我们更倾向于简单地使用cout或printf来debug。同时，我们之前封装的函数过于简单，对于debug来说，这样的输出函数显然是远远不够的。因此，我们接下来就要实现一个能够进行基本的格式化输出的printf函数。

但在实现printf之前，我们需要了解C语言的可变参数机制。

## 一个可变参数的例子

对于printf这类函数来说，我们可以使用任意数量的参数来调用printf，例如

```cpp
printf("hello world");
printf("this is a int: %d", 123);
printf("int, char, double: %d, %c, %f", 123, 'a', 3.14);
```

由于printf的函数参数并不是固定的，我们因此把这类函数称为可变参数函数。但在我们平时的编程当中，我们编写的函数都需要在函数头清晰地指出函数所需要的参数，也就是说，函数的参数在声明的时候就已经被固定下来了。那么printf是如何实现参数可变的呢？这就要用到C语言的可变参数机制。

C语言允许我们使用定义可变参数函数，此时函数的参数列表分为两部分，前一部分是固定下来的参数，如`int`，`char *`，`double`等用数据类型写出来的参数，称为“固定参数”；后一部分是“可变参数”，其使用`...`来表示。例如，printf的声明如下。

```cpp
int printf(const char* const _Format, ...);
```

printf的参数分为两部分，前一部分是字符串指针，表示需要格式化输出的字符串，后一部分是可变参数。

虽然我们可以方便地使用可变参数函数，但是我们在定义可变参数时需要遵循一些规则，如下所示。

1. 对于可变参数函数，参数列表中至少有一个固定参数。
2. 可变参数列表必须放在形参列表最后，也就是说，`...`必须放在函数的参数列表的最后，并且最多只有一个`...`。

明白了这些规则之后，我们就定义一个具有可变参数的函数`print_any_number_of_integers`（下面简称`panoi`），如下所示。

```cpp
void print_any_number_of_integers(int n, ...);
```

`panoi`顾名思义，就是用来输出若干个整数的函数。函数的参数分为两部分，`n`是可变参数的数量，`...`是可变参数，表示若干个待输出的整数。

在实现`panoi`前，我们首先学习如何在函数内部引用可变参数列表中的参数。

为了引用可变参数列表中的参数，我们需要用到`<stdarg.h>`头文件定义的一个变量类型`va_list`和三个宏`va_start`，`va_arg`，`va_end`，这三个宏用于获取可变参数列表中的参数，用法如下。

| 宏                                    | 用法说明                                                     |
| ------------------------------------- | ------------------------------------------------------------ |
| `va_list`                             | 定义一个指向可变参数列表的指针。                             |
| `void va_start(va_list ap, last_arg)` | 初始化可变参数列表指针`ap`，使其指向可变参数列表的起始位置，即函数的固定参数列表的最后一个参数`last_arg`的后面第一个参数。 |
| `type va_arg(va_list ap, type)`       | 以类型`type`返回可变参数，并使`ap`指向下一个参数。           |
| `void va_end(va_list ap)`             | 清零`ap`。                                                   |

为了引用可变参数列表的参数，我们也需要遵守一些规则，如下所示。

+ 可变参数必须从头到尾逐个访问。如果你在访问了几个可变参数之后想半途中止，这是可以的，但是，如果你想一开始就访问参数列表中间的参数，那是不行的(可以把想访问的中间参数之前的参数读取但是不使用，曲线救国)。
+ 这些宏是无法直接判断实际实际存在参数的数量。
+ 这些宏无法判断每个参数的类型，所以在使用`va_arg`的时候一定要指定正确的类型。
+ 如果在`va_arg`中指定了错误的类型，那么将会影响到后面的参数的读取。
+ 第一个参数也未必要是可变参数个数，例如`printf`的第一个参数就是字符串指针。

明白了上述规则后，我们现在来实现``panoi``，实现如下。

```cpp
void print_any_number_of_integers(int n, ...)
{
    // 定义一个指向可变参数的指针parameter
    va_list parameter;
    // 使用固定参数列表的最后一个参数来初始化parameter
    // parameter指向可变参数列表的第一个参数
    va_start(parameter, n);

    for ( int i = 0; i < n; ++i ) {
        // 引用parameter指向的int参数，并使parameter指向下一个参数
        std::cout << va_arg(parameter, int) << " ";
    }
    
    // 清零parameter
    va_end(parameter);

    std::cout << std::endl;
}
```

第4行，我们首先定义一个指向可变参数列表的指针`parameter`，`parameter`会帮助我们引用可变参数列表的参数。但是，此时`parameter`并未指向`function`的可变参数列表，我们需要使用`va_start`来初始化`parameter`，使其指向可变参数列表的第一个参数。为什么我们一定要指定一个固定参数呢？回想起第3章反复强调的C/C++函数调用规则——在函数调用前，函数的参数会被从右到左依次入栈。

# TODO 函数调用的图片

从上面的图片可以看到，无论参数数量有多少，这些参数都被统一地放到了栈上，只不过使用可变参数的函数并不知道这些栈上的参数具体含义。因此我们才需要使用`va_arg`来指定参数的类型后才能引用函数的可变参数。也就是说，只有到了函数的实现这一步才会知道可变参数放置的是什么内容。注意到栈的增长方式是从高地址向低地址增长的，因此函数的参数从左到右，地址依次增大。固定参数列表的最后一个参数的作用就是告诉我们可变参数列表的起始地址，如下所示。
$$
可变参数列表的起始地址=固定参数列表的最后一个参数的地址+这个参数的大小
$$
`va_start`就是实现了上面这条公式。

初始化了`parameter`后，我们就使用`parameter`和`va_arg`来引用可变参数。可变参数的函数并不知道每一个可变参数的类型和具体含义，它只是在调用前把这些参数放到了栈上。而我们人为地在`<stdarg.h>`中定义了一些访问栈地址的宏。我们可以指定这些参数的具体类型和使用这些宏来取出参数，这就是访问可变参数的实现思想。这也是为什么我们需要在`va_arg`中指明`parameter`指向的参数类型，因为只有函数实现者才知道函数想要的参数是什么。

从本质上来说，`parameter`就是指向函数调用栈的一个指针，类似`esp`、`ebp`，`va_arg`按照指定的类型来返回`parameter`指向的内容。注意，在`va_arg`返回后，`parameter`会指向下一个参数，无需我们手动调整。

访问完可变参数后，我们需要使用`va_end`对`parameter`进行清零，这是防止后面再使用`va_arg`和`parameter`来引用栈中的内容，从而导致错误。

然后我们来使用这个函数，如下所示。

```cpp
#include <iostream>
#include <stdarg.h>

void print_any_number_of_integers(int n, ...);

int main()
{
    print_any_number_of_integers(1, 213);
    print_any_number_of_integers(2, 234, 2567);
    print_any_number_of_integers(3, 487, -12, 0);
}

void print_any_number_of_integers(int n, ...)
{
    // 定义一个指向可变参数的指针parameter
    va_list parameter;
    // 使用固定参数列表的最后一个参数来初始化parameter
    // parameter指向可变参数列表的第一个参数
    va_start(parameter, n);

    for ( int i = 0; i < n; ++i ) {
        // 引用parameter指向的int参数，并使parameter指向下一个参数
        std::cout << va_arg(parameter, int) << " ";
    }
    
    // 清零parameter
    va_end(parameter);

    std::cout << std::endl;
}
```

编译运行。

```shell
g++ main.cpp -o main.out && ./main.out
```

输出如下结果。

```
213
234 2567
487 -12 0
```

学习完上面这个例子，我们对可变参数的机制已经有所了解，接下来我们就要实现可变参数的3个宏`va_start`、`va_arg`、`va_end`。

## 可变参数机制的实现

可变参数并不神秘，从本质上来说，C语言首先提供我们定义可变参数列表需要的符号`...`，有了这个符号后，我们可以任意地改变函数的形式参数，这些函数的参数都按顺序会被放到栈上面。此时，`<stdarg.h>`提供了3个访问栈中的参数的宏。而这些可变参数的具体意义是什么，需要我们在使用这些宏的时候人为规定。这便是可变参数的本质，我们现在就来实现它。

首先，`va_list`是指向可变参数列表的指针，其实就是字节类型的指针，而`char`类型就是1个字节，如下所示。

```cpp
typdef char * va_list;
```

然后，我们定义`va_start`，`va_start`是初始化一个指向可变参数列表起始地址的指针`ap`，需要用到固定参数列表的最后一个变量`v`，如下所示。

```cpp
#define _INTSIZEOF(n) ( (sizeof(n)+sizeof(int)-1) & ~(sizeof(int)-1) )
#define va_start(ap,v) ( ap = (va_list)&v + _INTSIZEOF(v) )
```

在保护模式下，栈中的push和pop的变量都是32位的，也就是4个字节。无论是`char`、`short`还是`int`，这些变量在栈中都是以4字节对齐的形式保存的。4字节对齐的意思是找到第一个不小于变量的长度且为4的倍数的整数。例如，`char`放在栈中时，虽然`char`是1个字节，但是栈使用4个字节来保存它。而ap是指向栈的，因此ap需要4个字节对齐，也就是ap的值需要是4的倍数。

对于一个整数$n$，其进行4字节对齐后得到的整数是$n'$，则有
$$
n'=(n+4-1)\div4\times4
$$
注意，上面的除法是整数除法。对于上面的公式，如果$n=2$，则有
$$
n'=(2+4-1)\div4\times4
=5\div4\times4
=1\times4
=4
$$
如果$n=4$，则有
$$
n'=(4+4-1)\div4\times4
=7\div4\times4
=1\times4
=4
$$
`_INTSIZEOF(n)`返回的是n的大小进行4字节对齐的结果。注意到，4的倍数在二进制表示中的低2位是0，而任何地址和`0xfffffffc`(`~(sizeof(int)-1)`)相与后得到的数的低2位为0，也就是4的倍数，即相当于上面公式除4再乘以4的过程。但是，直接拿一个数和`0xfffffffc`相与得到的结果是向下4字节对齐的，为了实现向上对齐，我们需要先加上`(sizeof(int)-1)`后再和`0xfffffffc`相与，此时得到的结果就是向上4字节对齐的。

`va_arg`的作用是返回`ap`指向的，`type`类型的变量，并同时使`ap`指向下一个参数，如下所示。

```cpp
#define va_arg(ap, type) ( *(type *)((ap += _INTSIZEOF(type)) - _INTSIZEOF(type)))
```

最后，`va_end`的作用是将`ap`清零，如下所示。

```cpp
#define va_end(ap) ( ap = (va_list)0 )
```

至此，可变参数的机制已经实现完毕，结合可变参数机制的实现过程，同学们应该能够对可变参数的使用有了进一步的理解。

下面我们就使用我们实现的宏来引用可变参数，如下所示。

```cpp
#include <iostream>

typedef char *va_list;
#define _INTSIZEOF(n) ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#define va_start(ap, v) (ap = (va_list)&v + _INTSIZEOF(v))
#define va_arg(ap, type) (*(type *)((ap += _INTSIZEOF(type)) - _INTSIZEOF(type)))
#define va_end(ap) (ap = (va_list)0)


void print_any_number_of_integers(int n, ...);

int main()
{
    print_any_number_of_integers(1, 213);
    print_any_number_of_integers(2, 234, 2567);
    print_any_number_of_integers(3, 487, -12, 0);
}

void print_any_number_of_integers(int n, ...)
{
    // 定义一个指向可变参数的指针parameter
    va_list parameter;
    // 使用固定参数列表的最后一个参数来初始化parameter
    // parameter指向可变参数列表的第一个参数
    va_start(parameter, n);
    
    for (int i = 0; i < n; ++i)
    {
        // 引用parameter指向的int参数，并使parameter指向下一个参数
        std::cout << va_arg(parameter, int) << " ";
    }

    // 清零parameter
    va_end(parameter);

    std::cout << std::endl;
}
```

编译运行。

```shell
g++ main.cpp -m32 -std=c++98 -o main.out && ./main.out
```

得到相同的结果。

```
213
234 2567
487 -12 0
```

此时，我们并未引入头文件`<stdarg.h>`，这说明了我们已经成功实现了可变参数机制。借助于我们实现的可变参数机制，我们接下来实现`printf`。

## 实现printf

学会了可变参数后，printf的实现便不再困难。在实现printf前，我们首先要明白printf的作用。printf的作用是格式化输出，并返回输出的字符个数，其定义如下。

```cpp
int printf(const char *const fmt, ...);
```

在格式化输出字符串中，会包含`%c,%d,%x,%s`等来实现格式化输出，对应的参数在可变参数中可以找到。明白了printf的作用，printf的实现便迎刃而解，实现思路如下。

printf首先找到fmt中的形如`%c,%d,%x,%s`对应的参数，然后用这些参数具体的值来替换`%c,%d,%x,%s`等，得到一个新的格式化输出字符串，这个过程称为fmt的解析。最后，printf将这个新的格式化输出字符串即可。然而，这个字符串可能非常大，会超过函数调用栈的大小。实际上，我们会定义一个缓冲区，然后对fmt进行逐字符地解析，将结果逐字符的放到缓冲区中。放入一个字符后，我们会检查缓冲区，如果缓冲区已满，则将其输出，然后清空缓冲区，否则不做处理。

在实现printf前，我们需要一个能够输出字符串的函数，这个函数能够正确处理字符串中的`\n`换行字符。这里，有同学会产生疑问，`\n`不是直接输出就可以了吗？其实`\n`的换行效果是我们人为规定的，换行的实现需要我们把光标放到下一行的起始位置，如果光标超过了屏幕的表示范围，则需要滚屏。因此，我们实现一个能够输出字符串的函数`STDIO::print`，声明和实现分别放在`include/stdio.h`和`src/kernel/stdio.cpp`中，如下所示。

```cpp
int STDIO::print(const char *const str)
{
    int i = 0;

    for (i = 0; str[i]; ++i)
    {
        switch (str[i])
        {
        case '\n':
            uint row;
            row = getCursor() / 80;
            if (row == 24)
            {
                rollUp();
            }
            else
            {
                ++row;
            }
            moveCursor(row * 80);
            break;

        default:
            print(str[i]);
            break;
        }
    }

    return i;
}
```

> 在程序设计中，命名是一件令人苦恼的事情，而借助于C++的函数重载，我们可以将许多功能类似的函数用统一的名字来表示。

我们实现的printf比较简单，只能解析如下参数。

| 符号 | 含义             |
| ---- | ---------------- |
| %d   | 按十进制整数输出 |
| %c   | 输出一个字符     |
| %s   | 输出一个字符串   |
| %x   | 按16进制输出     |

按照前面描述的过程，printf的实现如下。

```cpp
int printf(const char *const fmt, ...)
{
    const int BUF_LEN = 32;

    char buffer[BUF_LEN + 1];
    char number[33];

    int idx, counter;
    va_list ap;

    va_start(ap, fmt);
    idx = 0;
    counter = 0;

    for (int i = 0; fmt[i]; ++i)
    {
        if (fmt[i] != '%')
        {
            counter += printf_add_to_buffer(buffer, fmt[i], idx, BUF_LEN);
        }
        else
        {
            i++;
            if (fmt[i] == '\0')
            {
                break;
            }

            switch (fmt[i])
            {
            case '%':
                counter += printf_add_to_buffer(buffer, fmt[i], idx, BUF_LEN);
                break;

            case 'c':
                counter += printf_add_to_buffer(buffer, va_arg(ap, int), idx, BUF_LEN);
                break;

            case 's':
                buffer[idx] = '\0';
                idx = 0;
                counter += stdio.print(buffer);
                counter += stdio.print(va_arg(ap, const char *));
                break;

            case 'd':
            case 'x':
                int temp = va_arg(ap, int);

                if (temp < 0 && fmt[i] == 'd')
                {
                    counter += printf_add_to_buffer(buffer, '-', idx, BUF_LEN);
                    temp = -temp;
                }

                temp = itos(number, temp, (fmt[i] == 'd' ? 10 : 16));

                for (int j = temp - 1; j >= 0; --j)
                {
                    counter += printf_add_to_buffer(buffer, number[j], idx, BUF_LEN);
                }
                break;

            }
        }
    }

    buffer[idx] = '\0';
    counter += stdio.print(buffer);

    return counter;
}
```

首先我们定义一个大小为`BUF_LEN`的缓冲区`buffer`，`buffer`多出来的1个字符是用来放置`\0`的。由于我们后面会将一个整数转化为字符串表示，`number`使用来存放转换后的数字字符串。由于保护模式是运行在32位环境下的，最大的数字字符串也不会超过32位，因此number分配33个字节也就足够了。

接着我们开始对`fmt`进行逐字符解析，对于每一个字符`fmt[i]`，如果`fmt[i]`不是`%`，则说明是普通字符，直接放到缓冲区即可。注意，将`fmt[i]`放到缓冲区后可能会使缓冲区变满，此时如果缓冲区满，则将缓冲区输出并清空，我们不妨上述过程写成一个函数来实现，如下所示。

```cpp
int printf_add_to_buffer(char *buffer, char c, int &idx, const int BUF_LEN)
{
    int counter = 0;

    buffer[idx] = c;
    ++idx;

    if (idx == BUF_LEN)
    {
        buffer[idx] = '\0';
        counter = stdio.print(buffer);
        idx = 0;
    }

    return counter;
}
```

如果`fmt[i]`是`%`，则说明这可能是一个格式化输出的参数。因此我们检查`%`后面的参数，分为如下情况分别处理。

+ `%%`。输出一个`%`。
+ `%c`。输出`ap`指向的字符。
+ `%s`。输出`ap`指向的字符串的地址对应的字符串。
+ `%d`。输出`ap`指向的数字对应的十进制表示。
+ `%x`。输出`ap`指向的数字对应的16进制表示。
+ 其他。不做任何处理。

对于`%d`和`%x`，我们需要将数字转换为对应的字符串。一个数字向任意进制表示的字符串的转换函数如下所示，声明放置在`include/stdlib.h`中，实现放置在`src/utils/stdlib.cpp`中。

```cpp
/*
 * 将一个非负整数转换为指定进制表示的字符串。
 * num: 待转换的非负整数。
 * mod: 进制。
 * numStr: 保存转换后的字符串，其中，numStr[0]保存的是num的高位数字，以此类推。
 */

void itos(char *numStr, uint32 num, uint32 mod);
```

```cpp
void itos(char *numStr, uint32 num, uint32 mod) {
    numStr[0] = '\0';

    // 只能转换2~26进制的整数
    if (mod < 2 || mod > 26 || num < 0) {
        return;
    }

    uint32 length, temp;

    // 进制转换
    length = 0;
    while(num) {
        temp = num % mod;
        num /= mod;
        numStr[length] = temp > 9 ? temp - 10 + 'A' : temp + '0';
        ++length;
    }

    // 特别处理num=0的情况
    if(!length) {
        numStr[0] = '0';
        ++length;
    }

    // 将字符串倒转，使得numStr[0]保存的是num的高位数字
    for(int i = 0, j = length - 1; i < j; ++i, --j) {
        swap(numStr[i], numStr[j]);
    }
}
```

其中，`swap`函数也是声明在`include/stdlib.h`，实现在`src/utils/stdlib.cpp`中。

```cpp
template<typename T>
void swap(T &x, T &y);
```

```cpp
template<typename T>
void swap(T &x, T &y) {
    T z = x;
    x = y;
    y = z;
}
```

上述函数比较简单，我们不再赘述。

由于`itos`转换的是非负整数，对于`%d`的情况，如果我们输出的整数是负数，那么就要使用`itos`转换其相反数，在输出数字字符串前输出一个负号。

最后，当我们逐字符解析完`fmt`后，`buffer`中可能还会有未输出的字符，我们要将缓冲区的字符全部输出，返回输出的总字符`counter`。

接下来我们测试这个函数，我们在`setup_kernel`中加入对应的测试语句。

```cpp
#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;


extern "C" void setup_kernel()
{
    // 中断处理部件
    interruptManager.initialize();
    // 屏幕IO处理部件
    stdio.initialize();
    interruptManager.enableTimeInterrupt();
    interruptManager.setTimeInterrupt((void *)asm_time_interrupt_handler);
    //asm_enable_interrupt();
    printf("print percentage: %%\n"
           "print char \"N\": %c\n"
           "print string \"Hello World!\": %s\n"
           "print decimal: \"-1234\": %d\n"
           "print hexadecimal \"0x7abcdef0\": %x\n",
           'N', "Hello World!", -1234, 0x7abcdef0);
    //uint a = 1 / 0;
    asm_halt();
}

```

然后修改makefile。

```makefile
...

RUNDIR = ../run
BUILDDIR = build
INCLUDE_PATH = ../include


KERNEL_SOURCE = $(wildcard $(SRCDIR)/kernel/*.cpp)
CXX_SOURCE += $(KERNEL_SOURCE)
CXX_OBJ += $(KERNEL_SOURCE:$(SRCDIR)/kernel/%.cpp=%.o)

UTILS_SOURCE = $(wildcard $(SRCDIR)/utils/*.cpp)
CXX_SOURCE += $(UTILS_SOURCE)
CXX_OBJ += $(UTILS_SOURCE:$(SRCDIR)/utils/%.cpp=%.o)

ASM_SOURCE += $(wildcard $(SRCDIR)/utils/*.asm)
ASM_OBJ += $(ASM_SOURCE:$(SRCDIR)/utils/%.asm=%.o)

OBJ += $(CXX_OBJ)
OBJ += $(ASM_OBJ)

...
```

编译运行，输出如下结果。

```shell
make && make run
```

<img src="/home/nelson/NeXon/sysu-2021-spring-operating-system/lab5/第4章 二级分页机制/gallery/printf的实现.PNG" alt="printf的实现" style="zoom:38%;" />

至此，我们实现了一个简单的`printf`函数。

> 此后，我们就可以愉快地使用printf来debug啦~

# 内存管理的内容

# 内核线程

## 程序、进程和线程

程序、进程和线程的区别常常令人混淆，这里我们简要地区分一下。

+ 程序是指静态的、存储在文件系统上的、尚未运行的指令代码，它是实际运行时的程序的映像。

+ 进程是指正在运行的程序，即进行中的程序，程序必须在获得运行所需要的各类资源后才可以成为进程，资源包括进程所使用的栈，寄存器等。

+ 线程实际上是函数的载体，属于创建它的进程。进程创建的所有线程共享进程所拥有的全部资源。

## 用户线程和内核线程

用户线程指线程只由用户进程来实现，操作系统中无线程机制。在用户空间中实现线程的好处是可移植性强，由于是用户级的实现，所以在不支持线程的操作系统上也可以写出完美支持线程的用户程序。但是，用户线程存在以下缺点。

+ 若进程中的某个线程出现了阻塞， 操作系统不知道进程中存在线程，因此会将整个进程挂起，导致进程中的全部线程都无法运行。
+ 对于操作系统来说，调度器的调度单元是整个进程，并不是进程中的线程，所以时钟中断只能影响进程一级的执行流。当时钟中断发生后，操作系统的调度器只能感知到进程一级的调度实体，它要么把处理器交给进程 A，要么交给进程 B，绝不可能交给进程中的某个线程  。因此，但凡进程中的某个线程开始在处理器上执行后，只要该线程不主动让出处理器，此进程中的其他线程都没机会运行。同时， 由于整个进程占据处理器的时间片是有限的， 这有限的时间片还要再分给内部的线程，所以每个线程执行的时间片非常非常短暂，再加上进程内线程调度器维护线程表、运行调度算法的时间片消耗，反而抵销了内部调度带来的提速。  

所以，为了最大地发挥线程的价值，我们选择了在内核中实现线程机制。内核线程具有以下特点。

+ 相比在用户空间中实现线程，内核提供的线程相当于让进程多占了处理器资源，比如系统中运行有进程 A和一传统型进程B（无内核线程，用户线程机制）。假设进程A中显式地创建了3个线程，这样一来，进程A加上主线程便有了4个线程，加上进程B，在内核调度器眼中便有了5个独立的执行流。尽管其中4个线程都属于进程A，但对调度器来说这4个线程和进程一样被调度，因此调度器调度完一圈后，进程A使用了 80%的处理器资源，这才是真正的提速。
+ 另一方面的优点是当进程中的某一线程阻塞后，由于线程是由内核空间实现的，操作系统认识线程，所以就只会阻塞这一个线程，此线程所在进程内的其他线程将不受影响，这又相当于提速了。
+ 用户进程需要通过系统调用陷入内核，这多少增加了一些现场保护的栈操作，这还是会消耗一些处理器时间，但和上面的大幅度提速相比，这显然是微不足道的。  

## 线程的描述

线程的组成部分有操作系统为线程分配的栈，状态，优先级，运行时间，线程负责运行的函数，函数的参数等，这些组成部分被集中保存在一个结构中——PCB(Process Control Block)，如下所示，代码放在`include/thread.h`中。

```cpp
enum ThreadStatus
{
    CREATE,
    RUNNING,
    READY,
    BLOCKED,
    DEAD
};

struct PCB
{
    int *stack;                      // 栈指针，用于调度时保存esp
    char name[MAX_PROGRAM_NAME + 1]; // 线程名
    enum ThreadStatus status;        // 线程的状态
    int priority;                   // 线程优先级
    int pid;                       // 线程pid
    int ticks;                     // 线程时间片总时间
    int ticksPassedBy;             // 线程已执行时间
    ListItem tagInGeneralList; // 线程队列标识
    ListItem tagInAllList;     // 线程队列标识
};
```

我们来看PCB各成员的含义。

+ `stack`。各个内核线程是共享内核空间的，但又相对独立，这种独立性体现在每一个线程都有自己的栈。那么线程的栈保存在哪里呢？线程的栈就保存在线程的PCB中，我们会为每一个PCB分配一个页。上面的`struct PCB`只是这个页的低地址部份，线程的栈指针从这个页的结束位置向下递减，如下所示。

  <img src="/home/nelson/NeXon/sysu-2021-spring-operating-system/lab5/第5章 内核线程/gallery/TCB栈.png" alt="TCB栈" style="zoom:30%;" />

  因此，我们不能向线程的栈中放入太多的东西，否则当栈指针向下扩展时，会与线程的PCB的信息发生覆盖，最终导致错误。`stack`的作用是在线程被换下处理器时保存esp的内容，然后当线程被换上处理器后，我们用`stack`去替换esp的内容，从而实现恢复线程运行的效果。

+ `status`是线程的状态，如运行态、阻塞态和就绪态等。

+ `name`是线程的名称。

+ `priority`是线程的优先级，线程的优先级决定了抢占式调度的过程和线程的执行时间。

+ `pid`是线程的标识符，每一个线程的pid都是唯一的。

+ `ticks`是线程剩余的执行次数。在时间片调度算法中，每发生中断一次记为一个`tick`，当`ticks=0`时，线程会被换下处理器，然后将其他线程换上处理器执行。

+ `ticksPassedBy`是线程总共执行的`tick`的次数。

+ `tagInGeneralList`和`tagInAllList`是线程在线程队列中的标识，用于在线程队列中找到线程的PCB。

其中，`ListItem`是链表`List`中的元素的表示，如下所示，代码放置在`list.h`中。

```cpp
struct ListItem
{
    ListItem *previous;
    ListItem *next;
};
```

`List`是一个带头节点的双向链表，定义如下。

```cpp
class List
{
public:
    ListItem head;

public:
    // 初始化List
    List();
    // 显式初始化List
    void initialize();
    // 返回List元素个数
    int size();
    // 返回List是否为空
    bool empty();
    // 返回指向List最后一个元素的指针
    // 若没有，则返回nullptr
    ListItem *back();
    // 将一个元素加入到List的结尾
    void push_back(ListItem *itemPtr);
    // 删除List最后一个元素
    void pop_back();
    // 返回指向List第一个元素的指针
    // 若没有，则返回nullptr
    ListItem *front();
    // 将一个元素加入到List的头部
    void push_front(ListItem *itemPtr);
    // 删除List第一个元素
    void pop_front();
    // 将一个元素插入到pos的位置处
    void insert(int pos, ListItem *itemPtr);
    // 删除pos位置处的元素
    void erase(int pos);
    void erase(ListItem *itemPtr);
    // 返回指向pos位置处的元素的指针
    ListItem *at(int pos);
    // 返回给定元素在List中的序号
    int find(ListItem *itemPtr);
};
```

关于`List`的成员函数的实现同学们应该倒背如流，这里便不班门弄斧了，实现代码放置在`src/utils/list.cpp`中。

PCB相当于线程的身份证，只要掌握了PCB就能掌握线程的全部信息，下面我们来创建线程。

## 线程的创建

首先，我们创建一个程序调度器`ProgramManager`，用于线程和进程的调度，如下所示，代码放置在`include/program.h`中。

```cpp
#ifndef PROGRAM_H
#define PROGRAM_H

#include "list.h"
#include "thread.h"

class ProgramManager
{
public:
    List allPrograms;   // 所有状态的线程/进程的队列
    List readyPrograms; // 处于ready(就绪态)的线程/进程的队列
    PCB *running;       // 当前执行的线程
public:
    ProgramManager();
    void initialize();
    // 创建一个线程并放入就绪队列
    // 成功，返回pid；失败，返回-1
    int executeThread(ThreadFunction function, void *parameter, const char *name, int priority);
    // 执行一次调度
    void schedule();

public:
    //  分配一个未被占用的pid
    int allocatePid();
    // 按pid查找线程/进程
    PCB *findProgramByPid(int pid);
    // ListItem转换成PCB
    PCB *ListItem2PCB(ListItem *item);
};

void program_exit();

#endif
```

`allPrograms`是所有状态的线程和进程的队列，其中放置的是这些线程和进程的`tagInAllList`。`readyPrograms`是处在ready(就绪态)的线程/进程的队列，放置的是`tagInGeneralList`。

然后我们在`include/os_modules`中定义一个全局的`ProgramManager`，如下所示。

```cpp
#ifndef OS_MODULES_H
#define OS_MODULES_H

#include "interrupt.h"
#include "stdio.h"
#include "memory.h"
#include "program.h"

// 中断管理器
InterruptManager interruptManager;
// 输出管理器
STDIO stdio;
// 内存管理器
MemoryManager memoryManager;
// 进程/线程管理器
ProgramManager programManager;

#endif
```

在使用`ProgramManager`的成员函数前，我们必须初始化`ProgramManager`，如下所示，代码放置在`src/program/program.cpp`中。

```cpp
ProgramManager::ProgramManager()
{
    initialize();
}

void ProgramManager::initialize()
{
    allPrograms.initialize();
    readyPrograms.initialize();
    runningThread = nullptr;
}
```

现在我们来创建线程。

前面提到，线程是一个函数的载体，因此线程执行的代码实际上是一个函数的代码。那么线程可以执行哪些函数的代码呢？这里我们规定线程只能执行返回值为`void`，参数为`void *`的函数，其中，`void *`是函数参数的指针。因此，当我们有多个参数时，我们需要将其放入到一个结构体中，线程执行的函数类型声明如下。

```cpp
typedef void(*ThreadFunction)(void *);
```

线程的创建如下所示。

```cpp
int ProgramManager::executeThread(ThreadFunction function, void *parameter, const char *name, int priority)
{
    // 关中断，防止创建线程的过程被打断
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    // 分配一页作为PCB
    PCB *thread = (PCB *)memoryManager.allocatePages(AddressPoolType::KERNEL, 1);
    if (!thread)
        return -1;

    // 初始化分配的页
    memset((char *)thread, 0, PAGE_SIZE);

    for (int i = 0; i < MAX_PROGRAM_NAME && name[i]; ++i)
    {
        thread->name[i] = name[i];
    }

    thread->status = ThreadStatus::READY;
    thread->priority = priority;
    thread->ticks = priority * 10;
    thread->ticksPassedBy = 0;

    thread->pid = allocatePid();
    if (thread->pid == -1)
    {
        memoryManager.releasePages(AddressPoolType::KERNEL, (int)thread, 1);
        return -1;
    }

    // 线程栈
    thread->stack = (int *)((int)thread + PAGE_SIZE);
    thread->stack -= 7;
    thread->stack[0] = 0;
    thread->stack[1] = 0;
    thread->stack[2] = 0;
    thread->stack[3] = 0;
    thread->stack[4] = (int)function;
    thread->stack[5] = (int)program_exit;
    thread->stack[6] = (int)parameter;

    allPrograms.push_back(&(thread->tagInAllList));
    readyPrograms.push_back(&(thread->tagInGeneralList));

    // 恢复中断
    interruptManager.setInterruptStatus(status);

    return thread->pid;
}
```

由于我们现在来到的多线程的环境，诸如内存分配的工作实际上都需要进行线程互斥处理，但我们并没有实现线程互斥的工具如锁、信号量等，因此这里我们只是简单地使用关中断和开中断来实现线程互斥。为什么开/关中断有效呢？在后面可以看到，我们是在时钟中断发生时来进行线程调度的，因此关中断后，时钟中断无法被响应，线程就无法被调度直到再次开中断。只要线程无法被调度，那么线程的工作也就无法被其他线程打断，因此就实现了线程互斥。

和开/关中断等相关的的函数定义在`include/interrupt.h`中，如下所示。

```cpp
class InterruptManager
{

    ...

    // 开中断
    void enableInterrupt();
    // 关中断
    void disableInterrupt();
    // 获取中断状态
    // 返回true，中断开启；返回false，中断关闭
    bool getInterruptStatus();
    // 设置中断状态
    // status=true，开中断；status=false，关中断
    void setInterruptStatus(bool status);
    
    ...
};
```

函数的实现比较简单，放置在`src/interrupt/interrupt.cpp`中，这里便不再赘述。

关中断后，我们向内存管理器申请一页作为线程的PCB，若申请失败，则返回`-1`表示线程创建失败。否则，我们先将PCB清0，设置PCB的成员`name`、`status`、`priority`、`ticks`和`ticksPassedBy`。

然后我们为线程申请一个pid，申请pid的函数如下。

# TODO 用数组存取PID

```cpp
int ProgramManager::allocatePid()
{
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    // 0号线程
    if (allPrograms.empty())
        return 0;

    int pid = -1;
    PCB *program;

    for (int i = 0; i < MAX_PROGRAM_AMOUNT; ++i)
    {
        program = findProgramByPid(i);
        if (!program)
        {
            pid = i;
            break;
        }
    }

    interruptManager.setInterruptStatus(status);

    return pid;
}
```

`allocatePid`的基本实现思想是令pid从0到`MAX_PROGRAM_AMOUNT`（最大线程数量）开始遍历，检查pid是否和已有的线程的pid相同，若相同，则检查下一个pid，否则返回这个pid。检查pid是否相同是通过按pid查找线程的PCB的函数来完成的，这个函数如下所示。

```cpp
PCB *ProgramManager::findProgramByPid(int pid)
{
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    ListItem *item = allPrograms.head.next;
    PCB *program, *ans;

    ans = nullptr;
    while (item)
    {

        program = (PCB *)(((dword)item) & 0xfffff000);

        if (program->pid == pid)
        {
            ans = program;
            break;
        }
        item = item->next;
    }

    interruptManager.setInterruptStatus(status);

    return ans;
}
```

`findProgramByPid`遍历包含了所有线程的线程队列`allPrograms`，然后比较每一个线程的pid是否和给定的pid相同，若相同，则返回线程的PCB指针，否则返回nullptr。线程队列存储的不是各个线程的PCB，而是通过PCB的两个`ListItem`成员将它们串接成一个`List`。为什么通过PCB的`ListItem`在线程队列中保存了PCB的信息呢？我们知道，`ListItem`有两个`ListItem *`成员，`previous`和`next`。这两个成员分别保存了线程队列中上一个线程的`ListItem`的地址和下一个线程的`ListItem`的地址。由于PCB是一个页，起始地址的低12位必然为0(页的大小是4KB)，而PCB的`ListItem`成员是位于这个页当中的，因此，`ListItem`成员的地址和`0xfffff000`相与便可得到PCB的地址。知道了PCB的地址就可以访问PCB的其他成员，这也就知道了PCB的所有信息。

> 通过ListItem将PCB存储在线程队列中的做法比较巧妙，同学们注意理解和体会。

如果我们找不到一个可用的pid，我们就要返回-1表明线程创建失败。但在返回前，我们需要释放PCB占用的页内存。如果找到可用的pid，我们就初始化线程的栈。线程的栈是从PCB的顶部开始向下增长的，因此，线程栈的初始地址是PCB的起始地址加上页的大小。然后，我们在栈中放入一些数值。

+ 4个为0的值是要放到ebp，ebx，edi，esi中的。
+ `thread->stack[4]`是线程执行的函数的起始地址。
+ `thread->stack[5]`是线程的返回地址。
+ `thread->stack[6]`是线程的参数。

至于这4部份的作用我们在线程的调度中统一讲解。

创建完线程的PCB后，我们将其放入到`allPrograms`和`readyPrograms`中，等待时钟中断来的时候可以被调度上处理器。

最后我们将中断的状态恢复，此时我们便创建了一个线程。

## 线程的调度

我们首先要修改之前的处理时钟中断函数，如下所示。

```cpp
void c_time_interrupt_handler()
{
    PCB *cur = programManager.running;
    //printf("pid %d ticks: %d\n", cur->pid, cur->ticks);
    if (cur->ticks)
    {
        --cur->ticks;
        ++cur->ticksPassedBy;
    }
    else
    {
        programManager.schedule();
    }
    
}
```

这里，我们使用的是时间片轮转的线程调度算法。当时钟中断到来时，我们对当前线程的`ticks`减1，直到`ticks`等于0，然后执行线程调度。线程调度的代码如下。

```cpp
void ProgramManager::schedule()
{
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    if (readyPrograms.size() == 0)
    {
        interruptManager.setInterruptStatus(status);
        return;
    }

    if (running->status == ThreadStatus::RUNNING)
    {
        running->status = ThreadStatus::READY;
        running->ticks = running->priority * 10;
        readyPrograms.push_back(&(running->tagInGeneralList));
    }
    else if (running->status == ThreadStatus::DEAD)
    {
        memoryManager.releasePages(AddressPoolType::KERNEL, (int)running, 1);
    }

    PCB *cur = running->status == DEAD ? 0 : running;
    ListItem *item = readyPrograms.front();
    PCB *next = ListItem2PCB(item);
	 next->status = ThreadStatus::RUNNING;
    running = next;
    readyPrograms.pop_front();

    //printf("schedule: %x, %x\n", cur, next);

    asm_switch_thread(cur->stack, next->stack);

    interruptManager.setInterruptStatus(status);
}
```

首先，为了实现线程互斥，在进程线程调度前，我们需要关中断，退出时再恢复中断。接着，我们判断当前可调度的线程数量，如果`readyProgram`为空，那么说明当前系统中只有一个线程，因此无需进行调度。

否则，我们判断当前线程的状态，如果是运行态(RUNNING)，则重新初始化其状态为就绪态(READY)和`ticks`，并放入就绪队列，其他情况不做处理。然后定义一个指向被换下处理器的线程的指针`cur`。接着我们去就绪队列的第一个线程作为下一个执行的线程，从就绪队列中删去这个线程，设置其状态为运行态和当前正在执行的线程。

然后我们就开始将线程从`cur`切换到`next`，代码如下。线程的所有信息都在线程栈中，只要我们切换线程栈就能够实现线程的切换，线程栈的切换实际上就是将线程的栈指针放到esp中。

```asm
asm_switch_thread:
    push ebp
    push ebx
    push edi
    push esi

    mov eax, [esp + 5 * 4]
    mov [eax], esp ; 保存当前栈指针到PCB中，以便日后恢复

    mov eax, [esp + 6 * 4]
    mov esp, [eax] ; 此时栈已经从cur栈切换到next栈

    pop esi
    pop edi
    pop ebx
    pop ebp

    sti
    ret
```

首先我们保存寄存器`ebp`，`ebx`，`edi`，`esi`。为什么要保存这几个寄存器？这是由C语言的规则决定的，C语言要求被调函数主动为主调函数保存这4个寄存器的值。如果我们不遵循这个规则，那么当我们后面线程切换到C语言编写的代码时就会出错。然后，我们保存esp的值到线程的PCB中，用做下次恢复。注意，7-8行代码是首先将`cur->stack`的值放到`eax`中，然后向`[eax]`中写入`esp`的值，而`[eax]`等于`*(cur->stack)`，也就是将esp写入cur指向的线程的栈指针，此时，cur指向的PCB的栈结构如下。

<img src="/home/nelson/NeXon/sysu-2021-spring-operating-system/lab5/第5章 内核线程/gallery/cur图示.png" alt="cur图示" style="zoom:25%;" />

10-11行是将PCB的成员`stack`保存的线程栈指针的值写入到esp中，此时，`next`指向的线程有两种状态，一种是刚创建还未调度运行的，一种是之前被换下处理器现在又被调度。注意，由于esp发生了变化，此时的栈也就跟着发生变化。这两种状态对应的栈结构有些不一致，对于前者，其结构如下。

<img src="/home/nelson/NeXon/sysu-2021-spring-operating-system/lab5/第5章 内核线程/gallery/next第一种情况图示.png" alt="next第一种情况图示" style="zoom:25%;" />

接下来的`pop`语句会将4个0值放到esi，edi，ebx，ebp中，此时栈顶的数据是线程需要执行的函数的地址`function`。执行ret返回后，`function`会被加载进eip，从而是CPU跳转到这个函数中执行。此时，进入函数后，函数的栈顶是函数的返回地址，返回地址之上是函数的参数，符合函数的调用规则。而函数执行完成时，其执行ret指令后会跳转到返回地址`program_exit`，如下所示。

```cpp
void program_exit()
{
    PCB *thread = programManager.running;
    thread->status = ThreadStatus::DEAD;

    if (thread->pid)
    {
        programManager.schedule();
    }
    else
    {
        interruptManager.disableInterrupt();
        printf("halt\n");
        asm_halt();
    }
}
```

`program_exit`会将返回的线程的状态置为DEAD，然后调度下一个可执行的线程上处理器。注意，我们规定第一个线程是不可以返回的，这个线程的pid为0。

第二种情况是之前被换下处理器现在又被调度，其栈结构如下所示。

<img src="/home/nelson/NeXon/sysu-2021-spring-operating-system/lab5/第5章 内核线程/gallery/next第二种情况图示.png" alt="next第二种情况图示" style="zoom:25%;" />

执行4个`pop`后，之前保存在线程栈中的内容会被恢复到这4个寄存器中，然后执行ret后会返回调用`asm_switch_thread`的函数，也就是`ProgramManager::schedule`，然后在`ProgramManager::schedule`中恢复中断状态，返回到时钟中断处理函数，最后从时钟中断中返回，恢复到线程被中断的地方继续执行。

这样，通过`asm_switch_thread`中的`ret`指令和`esp`的变化，我们便实现了线程的调度。

> `asm_switch_thread`的设计比较巧妙，需要同学们结合函数的调用规则，线程栈的设计等知识综合分析。

至此，关于线程的内容我们已经实现完毕，接下来我们来编译运行。

## Assignment 3 第一个线程

> assignment 3：创建第一个线程，并输出“Hello World”，pid和线程的name。注意，第一个线程不可以返回。

代码在`src/kernel/setup.cpp`中，如下所示。

```cpp
void first_thread(void *arg)
{
    // 第1个线程不可以返回
    printf("pid %d name \"%s\": Hello World!\n", programManager.running->pid, programManager.running->name);
    asm_halt();
}

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

    // 进程/线程管理器
    programManager.initialize();

    // 创建第一个线程
    int pid = programManager.executeThread(first_thread, nullptr, "first thread", 1);
    if (pid == -1)
    {
        printf("can not execute thread\n");
        asm_halt();
    }

    ListItem *item = programManager.readyPrograms.front();
    PCB *firstThread = programManager.ListItem2PCB(item);
    firstThread->status = RUNNING;
    programManager.readyPrograms.pop_front();
    programManager.running = firstThread;
    asm_switch_thread(0, firstThread);

    asm_halt();
}
```

首先，我们新建一个线程。由于当前系统中没有线程，因此我们无法通过在时钟中断调度的方式将第一个线程换上处理器执行。因此我们的做法是找线程的PCB，然后手动执行类似`schedule`的过程，最后执行的`asm_switch_thread`会强制将第一个线程换上处理器执行。

最后我们编译运行，输出如下结果。

<img src="/home/nelson/NeXon/sysu-2021-spring-operating-system/lab5/第5章 内核线程/gallery/第一个线程.PNG" alt="第一个线程" style="zoom:38%;" />

至此，本章的内容已经讲授完毕。