
#include <gdbstub.h>
#include <gdbstub_x86_64.h>

#include "environment.h"
#include "hype_gdbstub.h"

namespace gdbstub {

static void write_char(const char ch) {
    const auto result = environment::serial2_write(ch);
    if (result.is_error()) {
        trace_result("gdbstub write_char failed", result);
        catastrophic_error("gdbstub failure");
    }
}

static char read_char() {
    const auto result = environment::serial2_read();
    if (result.is_error()) {
        trace_result("gdbstub read_char failed", result);
        catastrophic_error("gdbstub failure");
    }

    return result.value();
}

static bool available_char() {
    const auto result = environment::serial2_available();
    if (result.is_error()) {
        trace_result("gdbstub available_char failed", result);
        catastrophic_error("gdbstub failure");
    }

    return result.value();
}

static void store_regs(const hype::cpu_registers_t& registers, x86_64::registers_t& gdb_regs) {
    gdb_regs.rax = registers.rax;
    gdb_regs.rbx = registers.rbx;
    gdb_regs.rcx = registers.rcx;
    gdb_regs.rdx = registers.rdx;
    gdb_regs.r8 = registers.r8;
    gdb_regs.r9 = registers.r9;
    gdb_regs.r10 = registers.r10;
    gdb_regs.r11 = registers.r11;
    gdb_regs.r12 = registers.r12;
    gdb_regs.r13 = registers.r13;
    gdb_regs.r14 = registers.r14;
    gdb_regs.r15 = registers.r15;
    gdb_regs.rflags_full = registers.rflags;
    gdb_regs.rsi = registers.rsi;
    gdb_regs.rdi = registers.rdi;
    gdb_regs.rbp = registers.rbp;
    gdb_regs.rsp = registers.rsp;
    gdb_regs.rip = registers.rip;
    gdb_regs.cs = registers.cs;
    gdb_regs.ds = registers.ds;
    gdb_regs.es = registers.es;
    gdb_regs.fs = registers.fs;
    gdb_regs.gs = registers.gs;
    gdb_regs.ss = registers.ss;
}

static void load_regs(hype::cpu_registers_t& registers, const x86_64::registers_t& gdb_regs) {
    registers.rax = gdb_regs.rax;
    registers.rbx = gdb_regs.rbx;
    registers.rcx = gdb_regs.rcx;
    registers.rdx = gdb_regs.rdx;
    registers.r8 = gdb_regs.r8;
    registers.r9 = gdb_regs.r9;
    registers.r10 = gdb_regs.r10;
    registers.r11 = gdb_regs.r11;
    registers.r12 = gdb_regs.r12;
    registers.r13 = gdb_regs.r13;
    registers.r14 = gdb_regs.r14;
    registers.r15 = gdb_regs.r15;
    registers.rflags = gdb_regs.rflags_full;
    registers.rsi = gdb_regs.rsi;
    registers.rdi = gdb_regs.rdi;
    // registers.rbp = gdb_regs.rbp;
    // registers.rsp = gdb_regs.rsp;
    // registers.rip = gdb_regs.rip;
    // registers.cs = gdb_regs.cs;
    // registers.ds = gdb_regs.ds;
    // registers.es = gdb_regs.es;
    // registers.fs = gdb_regs.fs;
    // registers.gs = gdb_regs.gs;
    // registers.ss = gdb_regs.ss;
}

static void start_handling() {
    volatile bool reloaded = false;

    hype::cpu_registers_t registers{};
    asm_cpu_store_registers(&registers);
    if (reloaded) {
        return;
    }

    x86_64::registers_t gdb_regs{};
    store_regs(registers, gdb_regs);
    x86_64::enter_loop(gdb_regs);
    load_regs(registers, gdb_regs);

    // ReSharper disable once CppDFAUnusedValue
    reloaded = true;
    asm_cpu_store_registers(&registers);
}

void initialize() {
    initialize(read_char, write_char);
}

void handle_interrupt(const x86::interrupts::interrupt_t vector, hype::cpu_registers_t& registers) {
    x86_64::registers_t gdb_regs{};
    store_regs(registers, gdb_regs);

    x86_64::handle_exception(static_cast<unsigned int>(vector), gdb_regs);

    load_regs(registers, gdb_regs);
}

void start_handling_if_prompted() {
    gdbstub::reload_breakpoints();

    if (!available_char()) {
        return;
    }
    if (read_char() != 0x03) {
        return;
    }

    start_handling();
}

void wait_for_server() {
    const auto ch = read_char();
    if (ch != 0x03 && ch != '+') {
        return;
    }

    start_handling();
}

}
