#pragma once

#include <x86/vmx/vmx.h>
#include <x86/vmx/vmcs.h>

#include "base.h"
#include "context.h"


namespace hype {

status_t vmxon_for_vcpu(vcpu_t& cpu);

status_t setup_vmcs(context_t& context, vcpu_t& cpu);

}
