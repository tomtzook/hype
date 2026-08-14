#pragma once

#include <x86/vmx/vmx.h>
#include <x86/vmx/vmcs.h>

#include <base.h>
#include "../context.h"


namespace hype {

framework::result<uint64_t> vmread(x86::vmx::field_t field);
framework::result<> vmwrite(x86::vmx::field_t field, uint64_t value);

template<typename t_>
framework::result<t_> vmread(const x86::vmx::field_t field) {
    const auto val = verify(vmread(field));
    return framework::ok(t_{val});
}

void allow_msr_exit_in_bitmap(context_t& context, x86::msr::id_t msr_id, bool read, bool write);

framework::result<> vmxon_for_vcpu(vcpu_t& cpu);
framework::result<> setup_vmcs(context_t& context, vcpu_t& cpu);

framework::result<> emulate_fault_into_guest(x86::interrupts::interrupt_t vector, uint32_t error_code = 0);

}
