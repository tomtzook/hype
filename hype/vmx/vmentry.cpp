
#include "vmentry.h"

#include <x86/vmx/vmcs.h>
#include <debug.h>


extern "C" void on_vmentry_failure() {
    trace_error("vmentry failed to restore to guest rip");
}

extern "C" void on_vmresume_failure(const x86::vmx::instruction_result_t result) {
    switch (result) {
        case x86::vmx::instruction_result_t::vm_fail_invalid:
            trace_error("vmresume failed due to vmfailInvalid");
            break;
        case x86::vmx::instruction_result_t::vm_fail_valid: {
            const auto error_code = x86::vmx::vm_instruction_error();
            trace_error("vmresume failed due to vmfailValid[%a(%d)]", x86::vmx::instruction_error_str(error_code), error_code);
            break;
        }
        default:
            trace_error("vmresume failed due to unknown reason");
            break;
    }

    hype::do_vm_entry_checks();
}