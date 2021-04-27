void test_allocate()
{
    const int _KERNEL_PHYSICAL_START = 0x200000;
    const int _KERNEL_TOTAL_PAGE = 3840;
    const int _USER_PHYSICAL_START = 0x1100000;
    const int _USER_TOTAL_PAGE = 3840;
    const int _KERNEL_VIRTUAL_START = 0xc0100000;

    // 测试地址池
    {
        int a1 = memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 1);
        if (a1 != _KERNEL_PHYSICAL_START)
        {
            printf("kernel physical 1 failed!, address: 0x%x\n", a1);
            return;
        }

        int a2 = memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 8);
        if (a2 != _KERNEL_PHYSICAL_START + 1 * PAGE_SIZE)
        {
            printf("kernel physical 2 failed!, address: 0x%x\n", a2);
            return;
        }

        int a3 = memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 16);
        if (a3 != _KERNEL_PHYSICAL_START + 9 * PAGE_SIZE)
        {
            printf("kernel physical 3 failed!, address: 0x%x\n", a3);
            return;
        }

        memoryManager.releasePhysicalPages(AddressPoolType::KERNEL, a2, 8);
        int a4 = memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 2);
        if (a4 != _KERNEL_PHYSICAL_START + 1 * PAGE_SIZE)
        {
            printf("kernel physical 4 failed!, address: 0x%x\n", a4);
            return;
        }

        memoryManager.releasePhysicalPages(AddressPoolType::KERNEL, a1, 1);
        memoryManager.releasePhysicalPages(AddressPoolType::KERNEL, a3, 16);
        memoryManager.releasePhysicalPages(AddressPoolType::KERNEL, a4, 2);
        int a5 = memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 3840);
        if (a5 != _KERNEL_PHYSICAL_START)
        {
            printf("kernel physical 5 failed!, address: 0x%x\n", a5);
            return;
        }

        memoryManager.releasePhysicalPages(AddressPoolType::KERNEL, a5, 3840);
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    {
        int a1 = memoryManager.allocatePhysicalPages(AddressPoolType::USER, 1);
        if (a1 != _USER_PHYSICAL_START)
        {
            printf("user physical 1 failed!, address: 0x%x\n", a1);
            return;
        }

        int a2 = memoryManager.allocatePhysicalPages(AddressPoolType::USER, 8);
        if (a2 != _USER_PHYSICAL_START + 1 * PAGE_SIZE)
        {
            printf("user physical 2 failed!, address: 0x%x\n", a2);
            return;
        }

        int a3 = memoryManager.allocatePhysicalPages(AddressPoolType::USER, 16);
        if (a3 != _USER_PHYSICAL_START + 9 * PAGE_SIZE)
        {
            printf("user physical 3 failed!, address: 0x%x\n", a3);
            return;
        }

        memoryManager.releasePhysicalPages(AddressPoolType::USER, a2, 8);
        int a4 = memoryManager.allocatePhysicalPages(AddressPoolType::USER, 2);
        if (a4 != _USER_PHYSICAL_START + 1 * PAGE_SIZE)
        {
            printf("user physical 4 failed!, address: 0x%x\n", a4);
            return;
        }

        memoryManager.releasePhysicalPages(AddressPoolType::USER, a1, 1);
        memoryManager.releasePhysicalPages(AddressPoolType::USER, a3, 16);
        memoryManager.releasePhysicalPages(AddressPoolType::USER, a4, 2);
        int a5 = memoryManager.allocatePhysicalPages(AddressPoolType::USER, 3840);
        if (a5 != _USER_PHYSICAL_START)
        {
            printf("user physical 5 failed!, address: 0x%x\n", a5);
            return;
        }

        printf("user Physical test pass!\n");

        memoryManager.releasePhysicalPages(AddressPoolType::USER, a5, 3840);
    }
    ////////////////////////////////////////////////////////////////////////////////////////
    {
        int a1 = memoryManager.allocateVirtualPages(AddressPoolType::KERNEL, 1);
        if (a1 != _KERNEL_VIRTUAL_START)
        {
            printf("kernel virutal 1 failed!, address: 0x%x\n", a1);
            return;
        }

        int a2 = memoryManager.allocateVirtualPages(AddressPoolType::KERNEL, 8);
        if (a2 != _KERNEL_VIRTUAL_START + 1 * PAGE_SIZE)
        {
            printf("kernel virutal 2 failed!, address: 0x%x\n", a2);
            return;
        }

        int a3 = memoryManager.allocateVirtualPages(AddressPoolType::KERNEL, 16);
        if (a3 != _KERNEL_VIRTUAL_START + 9 * PAGE_SIZE)
        {
            printf("kernel virutal 3 failed!, address: 0x%x\n", a3);
            return;
        }

        memoryManager.releaseVirtualPages(AddressPoolType::KERNEL, a2, 8);
        int a4 = memoryManager.allocateVirtualPages(AddressPoolType::KERNEL, 2);
        if (a4 != _KERNEL_VIRTUAL_START + 1 * PAGE_SIZE)
        {
            printf("kernel virutal 4 failed!, address: 0x%x\n", a4);
            return;
        }

        memoryManager.releaseVirtualPages(AddressPoolType::KERNEL, a1, 1);
        memoryManager.releaseVirtualPages(AddressPoolType::KERNEL, a3, 16);
        memoryManager.releaseVirtualPages(AddressPoolType::KERNEL, a4, 2);
        int a5 = memoryManager.allocateVirtualPages(AddressPoolType::KERNEL, 3840);
        if (a5 != _KERNEL_VIRTUAL_START)
        {
            printf("kernel virutal 5 failed!, address: 0x%x\n", a5);
            return;
        }

        printf("Kernel virutal test pass!\n");

        memoryManager.releaseVirtualPages(AddressPoolType::KERNEL, a5, 3840);
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    {
        int a1 = memoryManager.allocatePages(AddressPoolType::KERNEL, 1);
        if (a1 != _KERNEL_VIRTUAL_START)
        {
            printf("allocate pages 1 failed!, address: 0x%x\n", a1);
            return;
        }
        else
        {
            memset((char *)a1, 0, 1 * PAGE_SIZE);
            printf("allocate pages 1 pass\n");
        }

        int a2 = memoryManager.allocatePages(AddressPoolType::KERNEL, 8);
        if (a2 != _KERNEL_VIRTUAL_START + 1 * PAGE_SIZE)
        {
            printf("allocate pages 2 failed!, address: 0x%x\n", a2);
            return;
        }
        else
        {
            memset((char *)a2, 0, 8 * PAGE_SIZE);
            printf("allocate pages 2 pass\n");
        }

        int a3 = memoryManager.allocatePages(AddressPoolType::KERNEL, 16);
        if (a3 != _KERNEL_VIRTUAL_START + 9 * PAGE_SIZE)
        {
            printf("allocate pages 3 failed!, address: 0x%x\n", a3);
            return;
        }
        else
        {
            memset((char *)a3, 0, 16 * PAGE_SIZE);
            printf("allocate pages 3 pass\n");
        }

        memoryManager.releasePages(AddressPoolType::KERNEL, a2, 8);
        int a4 = memoryManager.allocatePages(AddressPoolType::KERNEL, 2);
        if (a4 != _KERNEL_VIRTUAL_START + 1 * PAGE_SIZE)
        {
            printf("allocate pages 4 failed!, address: 0x%x\n", a4);
            return;
        }
        else
        {
            memset((char *)a4, 0, 2 * PAGE_SIZE);
            printf("allocate pages 4 pass\n");
        }

        memoryManager.releasePages(AddressPoolType::KERNEL, a3, 16);
        memoryManager.releasePages(AddressPoolType::KERNEL, a4, 2);
        memoryManager.releasePages(AddressPoolType::KERNEL, a1, 1);

        int a5 = memoryManager.allocatePages(AddressPoolType::KERNEL, 2000);
        if (a5 != _KERNEL_VIRTUAL_START)
        {
            printf("allocate pages 5 failed!, address: 0x%x\n", a5);
            return;
        }
        else
        {
            memset((char *)a5, 0, 9 * PAGE_SIZE);
            printf("allocate pages 5 pass\n");
        }

        printf("allocate pages test pass!\n");
    }
}