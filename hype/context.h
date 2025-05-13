#pragma once

#include <x86/paging/paging.h>
#include <x86/vmx/vmx.h>
#include <x86/vmx/controls.h>
#include <x86/atomic.h>

#include "cpu.h"
#include "memory.h"
#include "interrupts.h"
#include "environment.h"

namespace hype {

struct wanted_vm_controls_t {
    x86::vmx::pin_based_exec_controls_t pinbased;
    x86::vmx::processor_based_exec_controls_t procbased;
    x86::vmx::secondary_processor_based_exec_controls_t secondary_procbased;
    x86::vmx::vmexit_controls_t vmexit;
    x86::vmx::vmentery_controls_t vmentry;
};

struct vcpu_t {
    static constexpr size_t stack_size = 0x8000;
    page_aligned x86::vmx::vmstruct_t vmxon_region;
    page_aligned x86::vmx::vmstruct_t vmcs;
    page_aligned uint8_t msr_bitmap[x86::paging::page_size];
    page_aligned uint8_t host_stack[stack_size];
    cpu_registers_t context_registers;  // must be after stack, expected by other code
    bool is_in_vmx_operation;
};

struct context_t {
    page_aligned memory::gdt_t gdt;
    page_aligned interrupts::idt_t idt;
    page_aligned memory::page_table_t page_table;
    page_aligned memory::ept_t ept;

    x86::segments::tss64_t tss;
    x86::segments::gdtr_t gdtr;
    x86::interrupts::idtr_t idtr;

    vcpu_t cpus[environment::max_vcpu_supported];
    size_t cpu_count;
    wanted_vm_controls_t wanted_vm_controls;

    volatile uint8_t cpu_init_index;
};

extern context_t* g_context;

static inline vcpu_t& get_current_vcpu() {
    const auto id = environment::get_current_vcpu_id();
    return g_context->cpus[id];
}

}
