# lab1 编译内核/利用已有内核构建OS  

<center>
    任课老师：陈鹏飞
<br>    
    助教：陈泓仰、谢文欣、傅禹泽、张钧宇
</center>

# 实验要求

> + DDL：2021-03-11 12:00
> + 提交的内容：将**实验报告**和**源码**打包成一个压缩包，并命名为“lab1-学号-姓名”，然后提交到课程网站[http://course.dds-sysu.tech/course/3/homework]的“实验作业一”处。

1. **独立完成**实验5个部份**环境配置**、**编译Linux内核**、**Qemu启动内核并开启远程调试**、**制作Initramfs**和**编译并启动Busybox**。
2. 编写实验报告、结合实验过程和源码来谈谈你完成实验的过程和结果，提供程序运行截屏。
3. 我们根据同学们完成实验的数量和质量打分。如果同学们只是参考已有源码但是没有作任何修改，可以提交一个详细完整报告，除了要求2外，同学们需要说说实验为什么进行不下去，下一次实验可以做出哪些改变等，这样也可以得到基本分数。
4. 实验不限语言， C/C++/Rust都可以。
5. 实验不限平台， Windows、Linux和MacOS等都可以。
6. 实验不限CPU， ARM/Intel/Risc-V都可以。

# 实验概述

> 在本次实验中，同学们会熟悉现有Linux内核的编译过程和启动过程， 并在自行编译内核的基础上构建简单应用并启动。同时，同学们会利用精简的Busybox工具集构建简单的OS， 熟悉现代操作系统的构建过程。 此外，同学们会熟悉编译环境、相关工具集，并能够实现内核远程调试。

1. 搭建OS内核开发环境包括：代码编辑环境、编译环境、运行环境、调试环境等。
2. 下载并编译i386（32位）内核，并利用qemu启动内核。
3. 熟悉制作initramfs的方法。
4. 编写简单应用程序随内核启动运行。
5. 编译i386版本的Busybox，随内核启动，构建简单的OS。
6. 开启远程调试功能，进行调试跟踪代码运行。
7. 撰写实验报告。

# 环境配置

若在非Linux环境下，首先下载安装Virtualbox，用于启动虚拟
机。如果本身是Linux环境则不需要这个步骤。建议安装Ubuntu 18.04桌面版，并将下载源换成清华下载源。

在虚拟机下，安装如下工具。

1. 安装vscode以及在vscode中安装汇编、 C/C++插件。vscode将作为一个有力的代码编辑器。
2. 安装nasm。
3. 安装qemu-system-i386。
4. 安装cmake、 gdb工具。
5. 安装objdump, readelf。

# 编译Linux内核

1. 在 <https://www.kernel.org/> 下载内核5.10；
2. 将内核编译成i386 32位版本；
   1. 利用i386配置文件配置内核，该配置在
      `arch/x86/configs`下面；
   2. Terminal下输入`make i386_defconfig`
   3. Terminal下输入`make menuconfig` (安装libncurses5-dev)
   4. 开启debug。(compile the kernel with debug)
   5. 保存退出。
   6. Terminal下输入`Make – j4`（时间较长）。
   7. 找到linux压缩镜像： bzImage，一般在`arch/x86/boot`下面。

# Qemu启动内核并开启远程调试  

1. 使用`qemu`启动内核并开启远程调试。

   ```shell
   qemu -kernel bzImage -s -S -append "console=ttyS0" -nographic
   ```

2. Gdb装载vmlinux进行远程调试，break start_kernel;。

# 制作Initramfs  

1. 编写简单的C程序如HelloWorld。

2. 编译成32位可执行文件 （apt-get install libc6-devi386）。

3. cpio打包成initramfs。

   ```shell
   echo helloworld | cpio -o --format=newc > hwinitramfs
   ```

4. 启动内核，并加载initramfs。

   ```shell
   qemu -kernel bzImage -initrd hwinitramfs -append "console=ttyS0 rdinit=helloworld" -nographic  
   ```

# 编译并启动Busybox  

1. 在 <https://github.com/meefik/busybox>，下载Busybox。
2. 配置Busybox，静态编译成32位可执行文件。
3. Qemu启动内核并加载Busybox。

# 附录

## 校园网环境加速

部分软件在校园网下下载可能缓慢，建议使用镜像源。此处推荐两个：

- [Matrix 镜像源](https://mirrors.matrix.moe)（此镜像源位于校内）
- [清华 Tuna 镜像源](https://mirrors.tuna.tsinghua.edu.cn)

例如，

- <https://mirrors.matrix.moe/kernel/v5.x/> 的 `linux-5.6.12.tar.gz` 等压缩文件提供了 Linux Kernel 源代码的下载。
- 参照[帮助文档](https://mirrors.matrix.moe/docs/ubuntu)可以设置 Ubuntu 软件源更新使用 Matrix 镜像源。另外，Matrix 镜像源也提供 Arch Linux、Debian、CentOS、Fedora 等发行版的软件源镜像。

## busybox源码

如果github下载速度慢，可以从课程网站处下载。

```
http://files.dds-sysu.tech/wl/?id=YnIeNeKJ4LVb4bXQTxz4oCCuRs7WIBAr
```

