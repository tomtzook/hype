
#include <environment.h>

#include "x86/vmx/instrinsic.h"
#include "x86/vmx/environment.h"

#include "x86/vmx/vmcs.h"



x86::vmx::vmcs_t::vmcs_t() noexcept {
    m_loaded = false;
    clear();
}

uintn_t x86::vmx::vmcs_t::vmread(vmcs_field_t field) noexcept {
    if (!m_loaded) {
        // TODO: fail!
        return 0;
    }

    uintn_t value;
    // TODO: fail on error!
    CHECK_SILENT(_vmread(field, value));
    return value;
}

void x86::vmx::vmcs_t::vmwrite(vmcs_field_t field, uintn_t value) noexcept {
    if (!m_loaded) {
        // TODO: fail!
        return;
    }

    // TODO: fail on error!
    CHECK_SILENT(_vmwrite(field, value));
}

void x86::vmx::vmcs_t::clear() noexcept {
    CHECK_SILENT(initialize_vm_struct(m_data));
    CHECK_SILENT(_vmclear(environment::to_physical(&m_data)));
}
