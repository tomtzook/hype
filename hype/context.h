#pragma once

#include <x86/vmx/vmx.h>

#include "memory.h"
#include "environment.h"

namespace hype {

struct vcpu_t {
    x86::vmx::vmstruct_t vmxon_region page_aligned;
    x86::vmx::vmstruct_t vmcs page_aligned;
    bool is_in_vmx_operation;
};

struct context_t {
    memory::page_table_t page_table page_aligned;
    vcpu_t cpus[environment::max_vcpu_supported];
};

extern context_t* g_context;

static inline vcpu_t& get_current_vcpu() {
    const auto id = environment::get_current_vcpu_id();
    return g_context->cpus[id];
}

}
