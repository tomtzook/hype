
#include "x86/cpuid.h"

#include "x86/vmx/environment.h"


bool x86::vmx::is_supported() noexcept {
    // CPUID.1:ECX.VMX[bit 5] = 1 [SDM 3 23.6 P1050]
    x86::cpuid_eax01_t cpu_features{};
    x86::cpu_id(1, cpu_features);

    if(!cpu_features.ecx.bits.vmx) {
        return false;
    }

    return true;
}
