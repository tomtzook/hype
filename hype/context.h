#pragma once

#include <x86/paging/paging.h>
#include <x86/vmx/vmx.h>
#include <x86/vmx/controls.h>
#include <x86/atomic.h>

#include "memory.h"
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
    page_aligned x86::vmx::vmstruct_t vmxon_region;
    page_aligned x86::vmx::vmstruct_t vmcs;
    page_aligned uint8_t msr_bitmap[x86::paging::page_size];
    bool is_in_vmx_operation;
};

struct context_t {
    page_aligned memory::page_table_t page_table;
    page_aligned memory::ept_t ept;
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
