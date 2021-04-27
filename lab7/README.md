# 内存管理

# TODO

+ stdlib.h 代码注释, bitmap.h注释修改
+ bitmap解释不清楚
+ 

# 参考资料

# 实验概述

# 实验要求

# 内存的探查

ax = 0xe801

int 15h

# 位图

## 概念

位图，BitMap，是用于资源管理的一种数据结构。BitMap使用一位来和一个资源单元建立映射关系，例如使用1表示资源单元已经被分配，使用0表示资源单元空闲，如下所示。<img src="gallery/bitmap.png" alt="bitmap" style="zoom:40%;" />

使用BitMap可以以较小的代价实现对资源的管理。对于4GB的内存，在分页机制下，我们将资源单元定为一个物理页，大小为4KB。因此，使用BitMap来对该内存进行管理时，BitMap的大小为
$$
\frac{4GB}{8*4KB}=128KB.
$$
其内存占比为
$$
\frac{128KB}{4GB}*100\%=0.00305\%.
$$
可见使用BitMap来对资源管理时，其空间效率较高。下面我们来实现BitMap。

## BitMap的实现

从代码层面上去考察BitMap时，BitMap的成员包括

+ 一块存放BitMap的内存区域。
+ BitMap管理的资源单元数量。
+ 单独存取位和批处理存取位的方法。

如下所示，BitMap的定义放置在`include/bitmap.h`中。

```c++
class BitMap
{
public:
    // 被管理的资源个数，bitmap的总位数
    int length;
    // bitmap的起始地char址
    char *bitmap;
public:
    // 初始化
    BitMap();
    // 设置BitMap，bitmap=起始地址，length=总位数(被管理的资源个数)
    void initialize(char *bitmap, const int length);
    // 获取第index个资源的状态，true=allocated，false=free
    bool get(const int index) const;
    // 设置第index个资源的状态，true=allocated，false=free
    void set(const int index, const bool status);
    // 分配count个连续的资源，若没有则返回-1，否则返回分配的第1个资源单元序号
    int allocate(const int count);
    // 释放第index个资源开始的count个资源
    void release(const int index, const int count);
    // 返回Bitmap存储区域
    char *getBitmap();
    // 返回Bitmap的大小
    int size() const;
private:
    // 禁止Bitmap之间的赋值
    BitMap(const BitMap &) {}
    void operator=(const BitMap&) {}
};
```

为什么我们需要从外界向BitMap提供存储区域呢？这是因为我们使用BitMap来实现内存管理，所以我们无法使用动态内存分配来在BitMap的初始化函数中分配一个存储区域。在后面的实现中可以看到，我们会在内存中手动划分出一块区域来存储BitMap用来标识资源分配情况的位。

注意，BitMap的成员是有指针的。一般情况下，成员涉及指针的对象的赋值都需要使用动态内存分配获得一个新的指针，但我们还没有实现动态内存分配。所以，我们将`copy constructor`和`operator=`定义为private来禁止BitMap之间的直接赋值。这也是为什么我们在BitMap的初始化函数`initialize`中需要提供bitmap的存储区域。

根据BitMap的描述，我们不难实现BitMap的成员函数，如下所示，代码放置在`src/utils/bitmap.cpp`中。

```cpp
BitMap::BitMap()
{
    initialize(nullptr, 0);
}

void BitMap::initialize(char *bitmap, const int length)
{
    this->bitmap = bitmap;
    this->length = length;

    int bytes = ceil(length, 8);
    memset(bitmap, 0, bytes);

}

bool BitMap::get(const int index) const
{
    int pos = index / 8;
    int offset = index % 8;

    return (bitmap[pos] & (1 << offset));
}

void BitMap::set(const int index, const bool status)
{
    int pos = index / 8;
    int offset = index % 8;

    // 清0
    bitmap[pos] = bitmap[pos] & (~(1 << offset));

    // 置1
    if (status)
    {
        bitmap[pos] = bitmap[pos] | (1 << offset);
    }
}

int BitMap::allocate(const int count)
{
    if (count == 0)
        return -1;

    int index, empty, start;

    index = 0;
    while (index < length)
    {
        // 越过已经分配的资源
        while (index < length && get(index))
            ++index;

        // 不存在连续的count个资源
        if (index == length)
            return -1;

        // 找到1个未分配的资源
        // 检查是否存在从index开始的连续count个资源
        empty = 0;
        start = index;
        while ((index < length) && (!get(index)) && (empty < count))
        {
            ++empty;
            ++index;
        }

        // 存在连续的count个资源
        if (empty == count)
        {
            for (int i = 0; i < count; ++i)
            {
                set(start + i, true);
            }

            return start;
        }
    }

    return -1;
}

void BitMap::release(const int index, const int count)
{
    for (int i = 0; i < count; ++i)
    {
        set(index + i, false);
    }
}

char *BitMap::getBitmap()
{
    return (char *)bitmap;
}

int BitMap::size() const
{
    return length;
}
```

上面的代码逻辑比较清晰，因此这里只说明BitMap如何找到资源单元对应的状态位。

当我们我们使用指针来访问BitMap的存储区域时，最小的访问单元是字节，而资源单元的状态是使用一个位来表示的。因此，给定一个资源单元的序号$i$，我们无法通过`bitmap[i]`的方式来直接修改资源单元的状态。我们的做法是先定位到存储第$i$个资源单元的字节序号$pos$，然后再确定第$i$个资源单元的状态位在第$pos$字节中的偏移位置$offset$，最后使用位运算来修改该位即可，如下所示。
$$
i=8\cdot pos+offset,\ 0 \le offset<8
$$
注意，$offset$是从最低位开始算起的。

# 地址池

在开启分页机制之后，我们的内存就会被划分为一个个的固定长度的块，这个块被称为页。在保护模式下，一个页的大小是4KB。在程序设计的过程中，我们常常需要进行动态内存分配。我们会使用malloc来分配任意字节大小的内存，使用free来释放任意字节大小的内存。从我们目前的进度来看，实现以字节为粒度的内存管理方法并不怎么现实。但是，假如我们强制要求每次分配的内存大小只能是一个页，每次释放的内存大小也是一个页。这样的内存管理实现起来是简单的，因为我们只需要记录下内存中哪些页是已经被分配的，哪些页是空闲的就可以了。这种内存管理方法被称为页内存管理。在后面的章节中，我们将会学习如何使用页内存管理来实现以字节为粒度的内存管理，从而实现我们的malloc和free。这里，我们只需要实现页内存管理即可。

为了实现页内存分配，我们需要使用一种结构来标识地址空间中的哪些页是已经被分配的，哪些是未被分配的。这种结构被称为地址池。当需要页内存分配时，我们可以从地址池中取出一个空闲页。然后地址池便会标识该空闲页已被分配，最后计算并返回该空闲页对应的地址。空闲页只要没有被释放，就不会被再次分配。

注意到BitMap可用于资源的管理，地址池实际上使用了BitMap来对其地址空间进行管理。

从代码层面上来看，地址池的定义如下所示，代码放置在`include/address_pool.h`中。

```cpp
class AddressPool
{
public:
    BitMap resources;
    int startAddress;
public:
    AddressPool();
    // 初始化地址池
    void initialize(char *bitmap, const int length,const int startAddress);
    // 从地址池中分配count个连续页，成功则返回第一个页的地址，失败则返回-1
    int allocate(const int count);
    // 释放若干页的空间
    void release(const int address, const int amount);
};
```

第4-5行，地址池是对连续的若干个页进行管理的数据结构。所以我们会使用`resources`来标识地址池中哪些页是空闲的，哪些页是被分配的。我们使用`startAddress`来记录地址池管理的页的共同起始地址。假设每一页的大小为$PAGE\_SIZE$，那么地址池中的第$i$页的起始地址$address$为
$$
address=startAddress+i\times PAGE\_SIZE
$$
`AddressPool`的实现放在`src/utils/address_pool.cpp`中，如下所示。

```cpp
AddressPool::AddressPool()
{
}

// 设置地址池BitMap
void AddressPool::initialize(char *bitmap, const int length, const int startAddress)
{
    resources.initialize(bitmap, length);
    this->startAddress = startAddress;
}

// 从地址池中分配count个连续页
int AddressPool::allocate(const int count)
{
    uint32 start = resources.allocate(count);
    return (start == -1) ? -1 : (start * PAGE_SIZE + startAddress);
}

// 释放若干页的空间
void AddressPool::release(const int address, const int amount)
{
    resources.release((address - startAddress) / PAGE_SIZE, amount);
}
```

代码逻辑较为简单，这里便不再赘述。

# 物理内存管理

> 代码放置在`src/3`下。



# 分页机制



# 虚拟内存管理

