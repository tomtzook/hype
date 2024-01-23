#pragma once

#include <x86/vmx/vmx.h>
#include <x86/vmx/vmcs.h>

#include "base.h"
#include "context.h"


namespace hype {

status_t vmxon_for_vcpu(vcpu_t& cpu) noexcept;

status_t setup_vmcs(vcpu_t& cpu) noexcept;

}
