
#include "x86/error.h"

#include "x86/vmx/instrinsic.h"


extern "C" uint32_t _vmxon(void* vmxon_region) noexcept;
extern "C" uint32_t _vmxoff() noexcept;
extern "C" uint32_t _vmclear(void* vmcs) noexcept;
extern "C" uint32_t _vmread(uint32_t field, uintn_t* value) noexcept;
extern "C" uint32_t _vmwrite(uint32_t field, uintn_t value) noexcept;
extern "C" uint32_t _vmptrld(void* vmcs) noexcept;
extern "C" uint32_t _vmptrst(void* vmcs) noexcept;
extern "C" uint32_t _vmlaunch() noexcept;


// TODO: ADD BETTER CHECKS FOR INSTRUCTION ERRORS
inline static common::result vmx_result(uint32_t result) {
    if (!result) {
        return x86::result::INSTRUCTION_FAILED;
    }
    return common::result::SUCCESS;
}

common::result x86::vmx::_vmxon(physical_address_t address) noexcept {
    // VMXON [SDM 3 30.3 P1219]
    uint32_t result = ::_vmxon(&address);
    return vmx_result(result);
}

common::result x86::vmx::_vmxoff() noexcept {
    // VMXOFF [SDM 3 30.3 P1217]
    uint32_t result = ::_vmxoff();
    return vmx_result(result);
}

common::result x86::vmx::_vmclear(physical_address_t address) noexcept {
    // VMXOFF [SDM 3 30.3 P1203]
    uint32_t result = ::_vmclear(&address);
    return vmx_result(result);
}

common::result x86::vmx::_vmread(uint32_t field, uintn_t& value) noexcept {
    uint32_t result = ::_vmread(field, &value);
    return vmx_result(result);
}

common::result x86::vmx::_vmwrite(uint32_t field, uintn_t value) noexcept {
    uint32_t result = ::_vmwrite(field, value);
    return vmx_result(result);
}

common::result x86::vmx::_vmptrld(physical_address_t address) noexcept {
    uint32_t result = ::_vmptrld(&address);
    return vmx_result(result);
}

common::result x86::vmx::_vmptrst(void* address) noexcept {
    uint32_t result = ::_vmptrst(address);
    return vmx_result(result);
}

common::result x86::vmx::_vmlaunch() noexcept {
    uint32_t result = ::_vmlaunch();
    return vmx_result(result);
}
