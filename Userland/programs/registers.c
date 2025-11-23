#include "usrlib.h"

int registers_main(int argc, char * argv[]) {
    register_info_t regs;
    if (sys_regs(&regs) < 0) {
        print_err("No register snapshot available, press F1 to take one\n");
        return -1;
    }

    printf("RAX:    %p\n", regs.rax);
    printf("RBX:    %p\n", regs.rbx);
    printf("RCX:    %p\n", regs.rcx);
    printf("RDX:    %p\n", regs.rdx);
    printf("RBP:    %p\n", regs.rbp);
    printf("RDI:    %p\n", regs.rdi);
    printf("RSI:    %p\n", regs.rsi);
    printf("R8:     %p\n", regs.r8);
    printf("R9:     %p\n", regs.r9);
    printf("R10:    %p\n", regs.r10);
    printf("R11:    %p\n", regs.r11);
    printf("R12:    %p\n", regs.r12);
    printf("R13:    %p\n", regs.r13);
    printf("R14:    %p\n", regs.r14);
    printf("R15:    %p\n", regs.r15);
    printf("RIP:    %p\n", regs.rip);
    printf("CS:     %p\n", regs.cs);
    printf("RFLAGS: %p\n", regs.rflags);
    printf("RSP:    %p\n", regs.rsp);
    printf("SS:     %p\n", regs.ss);
    
    return 0;
}