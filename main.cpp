
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>

#include <stdio.h>

#include <thread>

void func()
{
    static int cnt = 0;
    printf("call func(). cnt=%d\n", cnt++);
}

void thread_run()
{
    while (1)
    {
        func();
        usleep(1000 * 500); // sleep 500ms
    }
}

void signal_handler(int mysignal, siginfo_t *si, void* arg)
{
    ucontext_t *context = (ucontext_t *)arg;
    context->uc_mcontext.gregs[REG_RIP] = context->uc_mcontext.gregs[REG_RIP] - 0x01; // $rip -= 0x01
}

int main()
{
    // install signal handler
    struct sigaction action;
    action.sa_sigaction = &signal_handler;
    action.sa_flags = SA_SIGINFO;
    sigaction(SIGTRAP, &action, NULL);

    // make memory writable
    uint64_t pagesize;
    pagesize = sysconf(_SC_PAGE_SIZE);
    void* func_addr = (void*)func;
    void* page_addr = (void*)((uint64_t)func_addr - ((uint64_t)func_addr % pagesize));
    mprotect(page_addr, pagesize, PROT_READ | PROT_WRITE | PROT_EXEC);

    // start thread for call func()
    std::thread t(thread_run);

    usleep(1000 * 1000 * 5); // sleep 5 sec

    // busy wait
    printf("install 0xCC...\n");
    uint8_t backup = *(uint8_t*)func_addr;
    *(uint8_t*)func_addr = 0xCC;

    printf("sleep 5 sec, now install function hook here is thread-safe...\n");
    usleep(1000 * 1000 * 5); // sleep 5 sec

    printf("uninstall 0xCC...\n");
    *(uint8_t*)func_addr = backup;

    t.join();
    return 0;
}