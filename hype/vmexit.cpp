
#include <x86/vmx/vmcs.h>
#include <x86/vmx/vmexit.h>

#include "base.h"
#include "cpu.h"

namespace hype {

status_t handle_vmexit(cpu_registers_t& registers) {
    CHECK_VMX(x86::vmx::vmread(x86::vmx::field_t::guest_rip, registers.rip));
    CHECK_VMX(x86::vmx::vmread(x86::vmx::field_t::guest_rsp, registers.rsp));
    CHECK_VMX(x86::vmx::vmread(x86::vmx::field_t::guest_rflags, registers.rflags));

    uint64_t exit_reason_raw;
    CHECK_VMX(x86::vmx::vmread(x86::vmx::field_t::exit_reason, exit_reason_raw));

    const auto exit_reason = static_cast<x86::vmx::exit_reason_t>(exit_reason_raw);

    TRACE_DEBUG("EXIT %d", exit_reason);

    // todo: handle exit

    // todo: resume

    return {};
}

}

extern "C" void vm_exit_handler(hype::cpu_registers_t& registers) {
    const auto status = handle_vmexit(registers);
    // todo: handle error

    //hype::debug::deadloop(); // TODO: this causes vm to pause (fault?)
    hype::hlt_cpu();
}
