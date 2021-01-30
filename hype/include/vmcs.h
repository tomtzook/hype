#pragma once

#include <x86/vmx/vmcs.h>

#include "common.h"
#include "vcpu.h"


namespace hype {

uintn_t get_cr0_host_mask() noexcept;
uintn_t get_cr4_host_mask() noexcept;

common::result setup_vmcs(hype::vcpu_t& cpu, x86::vmx::vmcs_t& vmcs) noexcept;

}
