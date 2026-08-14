
#include "cpu.h"

namespace hype {

void trace_regs(const cpu_registers_t& regs) {
    trace_debug("rax=0x%x, rbx=0x%x, rcx=0x%x, rdx=0x%x, rsi=0x%x, rdi=0x%x", regs.rax, regs.rbx, regs.rcx, regs.rdx, regs.rsi, regs.rdi);
    trace_debug("rip=0x%x, rsp=0x%x, rbp=0x%x, rflags=0x%x", regs.rip, regs.rsp, regs.rbp, regs.rflags);
    trace_debug("cs=0x%X, ds=0x%x, ss=0x%x, es=0x%x, gs=0x%x, fs=0x%x", regs.cs, regs.ds, regs.ss, regs.es, regs.gs, regs.fs);
}

void deadloop() {
    //trace_debug("Entering Deadloop");

    static volatile int wait = 1;
    while (wait) {
        __asm__ __volatile__("pause");
    }

    //trace_debug("Leaving Deadloop");
}

void hlt_cpu() {
    __asm__ volatile ("cli; hlt");
}

}
