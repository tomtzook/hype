#pragma once

#include <types.h>

#include "x86/vmx/environment.h"
#include "x86/vmx/operations.h"


namespace x86::vmx {

enum vmcs_field_t : uint32_t {

};

class vmcs_t {
public:
    vmcs_t() noexcept;

    uintn_t vmread(vmcs_field_t field) noexcept;
    void vmwrite(vmcs_field_t field, uintn_t value) noexcept;

    void clear() noexcept;

    friend common::result x86::vmx::read(vmcs_t& vmcs) noexcept;
    friend common::result x86::vmx::write(vmcs_t& vmcs) noexcept;
private:
    vm_struct_t m_data PAGE_ALIGNED;
    bool m_loaded;
};

}
