
#include <environment.h>

#include "x86/intrinsic.h"
#include "x86/cpuid.h"

#include "x86/memory.h"


static const x86::cpuid_t MAX_PHYSWIDTH_CPUID_LEAF = 0x80000008;

size_t x86::get_maximum_physical_address_width() noexcept {
    // "Software can determine a processor’s physical-address width by executing CPUID with 80000008H in EAX. The physical-address
    // width is returned in bits 7:0 of EAX" [SDM 3 24.11.5 P1079 "Notes"]

    x86::cpuid_regs_t cpuid_regs {};
    x86::cpu_id(MAX_PHYSWIDTH_CPUID_LEAF, cpuid_regs);

    return cpuid_regs.eax & 0xff;
}

bool x86::is_page_aligned(void* ptr) noexcept {
    uintn_t address = environment::to_physical(ptr);
    return is_page_aligned(address);
}

size_t x86::get_address_size(uintn_t address) noexcept {
    return _bit_scan_reverse(address) + 1;
}

bool x86::is_address_in_max_physical_width(uintn_t address) noexcept {
    size_t address_size = get_address_size(address);
    size_t maxphys_width = get_maximum_physical_address_width();

    return address_size <= maxphys_width;
}
