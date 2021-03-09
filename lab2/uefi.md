本文原始出自[UEFI-Tutorial](https://github.com/xfoxfu/UEFI-Tutorial/blob/master/1-Basic-Application.md)，为了方便校内访问，此处保持一份副本。如有疑问，欢迎询问傅禹泽（微信：xfoxfu）。

# 简单的 UEFI 应用程序

> **摘要**：本文将简单介绍 UEFI 的相关背景知识，随后提供一个绘制图形的 UEFI 应用程序。

## 计算机的启动

在「计算机组成原理」课程中，各位应该都编写过一个简单的 CPU，而这个 CPU 带有一段预先编写好的程序。在那时，我们的 CPU 只需要支持最简单的固定的外部设备，因此我们可以直接编写程序来满足这些外部设备的需要，并且把程序固化到 CPU 的 ROM 中。但是，现实中的计算机（指 IBM PC）具有多种不同的设备，而 CPU 需要适应多种外部设备和执行环境，因此人们发明了 DDR、PCI、PS/2、USB 等多种不同的通信协议，从而构建了 CPU 和外部设备通信的标准；同时，计算机通过引入 BIOS 这个中间层来实现设备的初始化。

在现在的计算机中，主板芯片组（如 AMD B450）会带有一块小型的 ROM，其中存放了初始化计算机的各种设备的代码。主板制造商通过连接 CPU 的地址线，使得芯片组上的程序的入口地址和 CPU 通电后的默认指令地址相同。这样，计算机在启动后就会执行这段代码，来检测内存、初始化主板芯片组、检测设备，而这段代码就被称为 BIOS。

在完成了基本的初始化过程后，BIOS 将加载磁盘扇区 0 的内容，放置在 `0x7C00` 地址，然后跳转到该地址执行引导程序。引导程序将完成探测内存布局、加载操作系统内核等工作，并最终进入到操作系统内核中。

这里叙述的引导过程有所简化，没有讨论在没有内存的情况下如何完成内存的初始化等内容。知乎用户[老狼](https://www.zhihu.com/people/mikewolfwoo)有一系列文章叙述了计算器 BIOS 和 UEFI 相关的知识，包括 Cache As RAM、可信启动等，各位读者可以简单参考作为补充。

## BIOS 与 UEFI

为了有效地让操作系统（引导程序）能够知晓系统的情况（如内存布局，也就是内存地址的分布情况），BIOS 向操作系统提供了一系列[系统调用](https://wiki.osdev.org/BIOS)，这些调用通过中断触发。当触发中断后，BIOS 之前在 CPU 中注册的中断处理程序就会被执行，从而执行对应的功能。

BIOS 本身没有任何的规范定义，而是各个厂家在漫长的历史实践中，建立了一系列约定，使得各个不同厂家生产的计算机能够获得相似的表现。但是，因为缺乏规范，这样的历史实践并不能保证在所有的计算机上表现一致。

此外，在 BIOS 启动的引导程序运行时，系统的内存布局存在一个特殊的约定，从而使得引导程序不必获得完整的内存布局就可以执行一些简单的工作。这段内存的大小为 1MB，对于较大的引导程序来说有些紧张。同时，一个扇区的大小只有 512B，这也限制了引导程序的代码大小。因此，现在的引导扇区程序（如 Grub）往往采用两阶段引导的模式，第一阶段称为 Boot，放置在引导扇区中；而第二阶段称为 Loader，放置在磁盘的一个特殊区域；从而突破 BIOS 的代码和内存限制。

最后，BIOS 基于中断的函数调用产生了两个问题。其一是，中断调用的效率较低，导致计算机启动缓慢。其二是，引导程序和操作系统为了实现高级功能，往往会注册自己的中断处理程序，这可能和 BIOS 发生冲突。

为了更好地解决这一问题，Intel 联合 PC 厂商建立了 UEFI 标准，这是下一代的计算机固件的接口定义。有了 UEFI，操作系统开发人员能够更好地去实现操作系统的功能，编写更加复杂的引导程序，甚至可以在引导程序或固件管理界面访问网络、显卡等高级功能。

## UEFI 程序

在 BIOS 引导中，我们将程序编译为裸二进制，放置在引导扇区中，这样的操作非常复杂，而且不方便调试。对于 UEFI 引导，我们只需要将程序编译为 EFI 格式的可执行文件，放置在磁盘的 EFI 分区的特定位置，即可令 UEFI 加载我们的可执行文件。

因此，编写 UEFI 应用程序和编写一般的应用程序的差别不是很大，我们同样会运用以前学习过的编译、链接等方法生成二进制文件。只是，这个二进制文件的格式和我们一般的在操作系统上运行的格式不同，因此我们需要添加一些编译参数。当然，如果利用一些现有的工具方法，那么这部分工作也有前人帮助我们完成了。

对于传统的 C/C++ 应用程序，我们会编写

```cpp
int main(int argc, char** argv);
```

而对于 EFI 应用程序来说，程序的入口稍有不同。它的入口定义为

```cpp
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
```

其中，`SystemTable` 是我们访问 UEFI 功能的核心，其中包含了一些数据（如系统固件的名称和版本）以及一些函数指针，这些函数指针将提供众多的 UEFI 功能，以及和其它 UEFI 应用程序交互的方法。是的，UEFI 中，各种设备的驱动也作为 UEFI 应用程序提供。

> **提示**：从这里可以看到，UEFI 应用程序和普通的运行在操作系统上的应用程序是类似的，而且其同样通过入口点的参数来传递系统的相关信息。

常见的编写 C/C++ UEFI 应用程序的框架包括

- [EDK Ⅱ](https://github.com/tianocore/edk2)
- [gnu-efi](https://sourceforge.net/projects/gnu-efi)
- [POSIX-UEFI](https://gitlab.com/bztsrc/posix-uefi)

这些框架提供了标准的构建流程、类型声明以及辅助函数，从而使得我们可以方便地编写 UEFI 应用程序。笔者这里推荐使用 POSIX-UEFI 库，因为其和传统的 libc 提供了类似的函数，并且提供了预置的 Makefile，构建起来非常方便。

> **提示**：需要注意的是，因为 UEFI 运行时没有操作系统，因此也就没有 libc，所以我们不能直接使用 `printf` 一类的函数。这也是我们需要使用开发框架的原因。

对于 Rust 语言，则推荐使用 [uefi](https://lib.rs/crates/uefi) 库。

## 图形绘制应用程序

> **TODO**：关于 C++ 的版本暂无代码提供，可以参考 [POSIX-UEFI](https://gitlab.com/bztsrc/posix-uefi/-/tree/master/examples) 的例子。其中，例子 1 提供了构建 UEFI 应用程序的示例，例子 9 提供了关于 GOP 访问的一些实例。

### SystemTable

对于 UEFI，所有的信息均通过 SystemTable 提供。SystemTable 定义为

```cpp
typedef struct {
    efi_table_header_t              Hdr;

    wchar_t                         *FirmwareVendor;
    uint32_t                        FirmwareRevision;

    efi_handle_t                    ConsoleInHandle;
    simple_input_interface_t        *ConIn;

    efi_handle_t                    ConsoleOutHandle;
    simple_text_output_interface_t  *ConOut;

    efi_handle_t                    ConsoleErrorHandle;
    simple_text_output_interface_t  *StdErr;

    efi_runtime_services_t          *RuntimeServices;
    efi_boot_services_t             *BootServices;

    uintn_t                         NumberOfTableEntries;
    efi_configuration_table_t       *ConfigurationTable;
} efi_system_table_t;
```

对于 POSIX-UEFI，可以通过全局变量 `ST` 访问 SystemTable。对于 gnu-efi，其通过 `efi_main` 函数的第二个参数提供。

SystemTable 提供的服务可以分为两个部分，`RuntimeServices` 和 `BootServices`。RuntimeServices 是 UEFI 在操作系统运行时提供的服务，包括一些基础的时钟、内存布局等信息。BootServices 是 UEFI 在机器的引导阶段（未进入操作系统时）提供的服务，包括磁盘读取、内存分配、硬件驱动等功能，使得引导程序「几乎」和操作系统上运行没有区别。

此外，SystemTable 提供了输入输出功能，对于有文本终端的设备（比如树莓派），其将利用文本终端；对于没有文本终端的设备（比如我们的计算机），会利用键盘和显示器进行模拟输入输出。

要进行文本的输入输出，我们可以直接使用这里的输入输出调用，也可以利用 gnu-efi 或者 POSIX-UEFI 提供的输入输出函数（如 `printf`）进行输入输出。Rust 的 uefi 库提供了和标准库一致的输入输出接口。

### Protocol

要进行图形绘制，我们需要调用 UEFI 中的驱动程序。驱动程序在 UEFI 中被称为 Protocol，通过 GUID 进行标识。GUID 是强类型的，也就是说相同的 GUID 均对应相同的一个 Protocol，如 `EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID` 就对应了 `efi_gop_t`。

我们通过 BootServices 的 `LocateProtocol` 获得 Protocol 的指针。例如，POSIX-UEFI 中可以编写

```cpp
efi_status_t status;
efi_guid_t gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
efi_gop_t *gop = NULL;
status = BS->LocateProtocol(&gopGuid, NULL, (void**)&gop);
```

从而获得 GOP Protocol，这也就是基本图形绘制驱动。

对于 Rust uefi 库，我们也可以类似地实现

```rust
uefi_services::init(&st).expect_success("failed to initialize utilities");

let bs = st.boot_services();

let gop = bs
    .locate_protocol::<GraphicsOutput>()
    .expect_success("failed to get GraphicsOutput");
let gop = unsafe { &mut *gop.get() };
```

其它繁多的 UEFI 功能均以 Protocol 的形式提供，各位读者可以自行探索。例如，需要访问文件系统可以使用 `EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID`，当然，对于 POSIX-UEFI 可以使用和普通 C/C++ 程序类似的 `fopen`。

### GOP 显存

UEFI 中提供了一个非常初等的显示驱动，称为 GOP (GraphicsOutput)。和 BIOS 的 VGA/BGA/TGA 显存放置在内存的特定位置不同，GOP 的显存位置是不确定的，需要通过函数调用获得。这个显示驱动的性能较差，但对于我们的引导程序以及操作系统原型来说足够了，否则就需要初始化 PCI 总线并编写显卡驱动，这显然超出了操作系统课程的范围。

前面已经介绍过了 Rust 语言获得 GOP 驱动的方法，下面我们来获得一些信息：

```rust
let mode = gop.current_mode_info();
let (display_x, display_y) = mode.resolution();
let (display_x, display_y) = (display_x as isize, display_y as isize);
let fb_addr = gop.frame_buffer().as_mut_ptr() as u64;
let fb_size = gop.frame_buffer().size() as u64;
```

这里我们分别获得了显示模式、分辨率和显存地址、大小。显存被称为 `frame_buffer`。

显存是一个矩阵，存储了各个点的像素值，按照行集中存储。因此，用下面的代码，我们可以修改像素的颜色

```rust
let stride = mode.stride();
unsafe {
    *(fb_addr as *mut u32)
        .offset(row * stride + col)
        .as_mut()
        .unwrap() = COLORS[color];
}
```

其中，`stride` 是行存储的大小，可能比像素数略大，因为要考虑到元素对其以提高性能。彩色像素的格式由 `mode.pixel_format()` 指定，例如 `PixelFormat::Rgb` 代表「Each pixel is 32-bit long, with 24-bit RGB, and the last byte is reserved.」

### 构建

UEFI 应用程序的构建是比较容易的，对于使用 C++ 的读者，我们可以参考 POSIX-UEFI 的[例子]((https://gitlab.com/bztsrc/posix-uefi/-/blob/master/examples/01_helloworld/Makefile)，编写 Makefile 来构建。

对于 Rust 语言，我们则直接利用 cargo 进行构建，并且利用 Makefile 来生成 UEFI 所需要的文件结构，可以参考 [rust-xos](https://github.com/xfoxfu/rust-xos/tree/2cc61ad6d3a894801cb74d23d55de2a52f1cd007)。

我们利用 QEMU 来执行程序，利用好 QEMU 提供的直接将本地路径作为虚拟磁盘的方法，可以这样运行 UEFI 程序：

```bash
mkdir -p $(ESP)/EFI/Boot
cp $(EFI) $(ESP)/EFI/Boot/BootX64.efi
qemu-system-x86_64 \
    -bios ${OVMF} \
    -drive format=raw,file=fat:rw:${ESP}
```

其中需要用到 OVMF 来模拟 EFI 环境，这可以从 [tianocore 的网站](https://sourceforge.net/projects/edk2/files/OVMF/) 获得。

此外，也可以利用 `uefi-run` 来执行，安装方法为：

```bash
cargo install uefi-run
uefi-run -b /path/to/OVMF.fd -q $(which qemu) app.efi
```

<!-- ## 附录：关于 arm 指令集 -->
