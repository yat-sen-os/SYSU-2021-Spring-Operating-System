void third_thread(void *arg)
{
    printf("pid %d parameter %x: Third Thread!\n", programManager.running->pid, *((int *)arg));
    printf("third thread return\n");
}
void second_thread(void *arg)
{
    printf("pid %d name %s: Second Thread!\n", programManager.running->pid, programManager.running->name);
    int parameter = 0xabcdef;
    int pid = programManager.executeThread(third_thread, &parameter, "third thread", 1);
    if (pid == -1)
    {
        printf("can not executed thread 3\n");
    }

    while (1)
    {
        printf("2\n");
    }
}

void first_thread(void *arg)
{
    printf("status: %d\n", interruptManager.getInterruptStatus());
    printf("pid %d name \"%s\": Hello World!\n", programManager.running->pid, programManager.running->name);
    int pid = programManager.executeThread(second_thread, nullptr, "second thread", 1);
    if (pid == -1)
    {
        printf("can not execute thread!\n");
    }
    while (1)
    {
        // interruptManager.disableInterrupt();
        printf("1\n");
        // interruptManager.enableInterrupt();
    }
}