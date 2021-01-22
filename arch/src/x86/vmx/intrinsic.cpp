
#include "x86/error.h"

#include "x86/vmx/instrinsic.h"


extern "C" uint32_t _vmxon(void* vmxon_region) noexcept;
extern "C" uint32_t _vmxoff() noexcept;
extern "C" uint32_t _vmclear(void* vmcs) noexcept;


common::result x86::vmx::_vmxon(physical_address_t address) noexcept {
    // VMXON [SDM 3 30.3 P1219]
    uint32_t result = ::_vmxon(&address);
    return result ? x86::result::SUCCESS : x86::result::INSTRUCTION_FAILED;
}

common::result x86::vmx::_vmxoff() noexcept {
    // VMXOFF [SDM 3 30.3 P1217]
    uint32_t result = ::_vmxoff();
    return result ? x86::result::SUCCESS : x86::result::INSTRUCTION_FAILED;
}

common::result x86::vmx::_vmclear(physical_address_t address) noexcept {
    uint32_t result = ::_vmclear(&address);
    return result ? x86::result::SUCCESS : x86::result::INSTRUCTION_FAILED;
}
