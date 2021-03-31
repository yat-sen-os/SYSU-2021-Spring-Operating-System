# 实验要求

+ gdb查看可变参数的例子。

  

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
\begin{aligned}
n'&=(2+4-1)\div4\times4\\
&=5\div4\times4\\
&=1\times4\\
&=4
\end{aligned}
$$
如果$n=4$，则有
$$
\begin{aligned}
n'&=(4+4-1)\div4\times4\\
&=7\div4\times4\\
&=1\times4\\
&=4
\end{aligned}
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


void function(int n, ...);

int main()
{
    function(1, 213);
    function(2, 234, 2567);
    function(3, 487, -12, 0);
}

void function(int n, ...)
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

此时，我们并未引入头文件`<stdarg.h>`，这说明了我们已经成功实现了可变参数机制。

## 实现printf

学会了可变参数后，printf的实现便不再困难。在实现printf前，我们首先要明白printf的作用。printf的作用是格式化输出，并返回输出的字符个数，其定义如下。

```cpp
int printf(const char *const fmt, ...);
```

在格式化输出字符串中，会包含`%c,%d,%x,%s`等来实现格式化输出，对应的参数在可变参数中可以找到。明白了printf的作用，printf的实现便迎刃而解，实现思路如下。

printf首先找到fmt中的形如`%c,%d,%x,%s`对应的参数，然后用这些参数具体的值来替换`%c,%d,%x,%s`等，得到一个新的格式化输出字符串，称为fmt的解析，最后将这个新的格式化输出字符串即可。但是，这个字符串可能非常大，会超过函数调用栈的大小。因此我们实际上会定义一个缓冲区，然后对fmt进行逐字符地解析，将结果逐字符的放到缓冲区中。放入一个字符后，我们会检查缓冲区，如果缓冲区已满，则将其输出，然后清空缓冲区，否则不做处理。

在实现printf前，我们需要一个能够输出字符串的函数，这个函数能够正确处理字符串中的`\n`换行字符。这里，有同学会产生疑问，`\n`不是直接输出就可以了吗？其实`\n`的换行效果是我们人为规定的，换行的实现需要我们把光标放到下一行的起始位置，如果光标超过了屏幕的表示范围，则需要滚屏。因此，我们实现一个能够输出字符串的函数`STDIO::print`，声明和实现分别放在`include/stdio.h`和`src/io/stdio.cpp`中，如下所示。

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

printf的实现如下。

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

首先我们定义一个大小为`BUF_LEN`的缓冲区`buffer`，`buffer`多出来的1个字符是用来放置`\0`的。由于我们后面会将一个整数转化为字符串表示，`number`使用来存放转换后的字符串的。

接着我们开始对`fmt`进行逐字符解析，对于每一个字符`fmt[i]`，如果`fmt[i]`不是`%`，则说明是普通字符，直接放到缓冲区即可。注意，将`fmt[i]`放到缓冲区后可能会使缓冲区变满，此时如果缓冲区满，则将缓冲区输出并清空，上述过程写成一个函数来实现，如下所示。

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

一个数字向任意进制表示的字符串的转换函数如下所示，声明放置在`include/stdlib.h`中，实现放置在`src/utils/stdlib.cpp`中。

```cpp
int itos(char *numStr, int num, int mod) {
    if (mod < 2 || mod > 26 || num < 0) {
        return -1;
    }

    int length, temp;

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

    return length;
}
```

最后，当我们逐字符解析完`fmt`后，`buffer`中可能还会有未输出的字符，我们要将其输出，然后返回输出的总字符`counter`。

接下来我们测试这个函数，我们在`setup_kernel`中加入对应的测试语句。

```cpp
#include "os_type.h"
#include "os_modules.h"
#include "asm_utils.h"
#include "stdio.h"
#include "stdlib.h"

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

然后编译运行，输出如下结果。

<img src="/home/nelson/NeXon/sysu-2021-spring-operating-system/lab5/第4章 二级分页机制/gallery/printf的实现.PNG" alt="printf的实现" style="zoom:38%;" />

至此，我们实现了一个简单的`printf`函数。

> 此后，我们就可以愉快地使用printf来debug啦~
