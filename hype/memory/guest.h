#pragma once

#include <x86/vmx/ept.h>
#include <base.h>

namespace hype::memory {

framework::result<x86::vmx::ept_pointer_t> read_current_eptp();
framework::result<x86::cr3_t> read_current_guest_cr3();
framework::result<x86::paging::mode_t> read_current_guest_paging_mode();

framework::result<physical_address_t> gpa_to_hpa(const x86::vmx::ept_pointer_t& eptp, uint64_t address);
framework::result<physical_address_t> gva_to_gpa_ia32(const x86::vmx::ept_pointer_t& eptp, const x86::cr3_t& guest_cr3, uint64_t address);
framework::result<physical_address_t> gva_to_gpa(const x86::vmx::ept_pointer_t& eptp, x86::paging::mode_t guest_mode, const x86::cr3_t& guest_cr3, uint64_t address);
framework::result<physical_address_t> gva_to_hpa(const x86::vmx::ept_pointer_t& eptp, x86::paging::mode_t guest_mode, const x86::cr3_t& guest_cr3, uint64_t address);

struct frame_range {
    uint32_t index;
    uint32_t count;
};

struct frame_ranges {
    frame_range ranges[256];
    size_t count = 0;
};

framework::result<frame_ranges> load_guest_ranges(const x86::vmx::ept_pointer_t& eptp, x86::paging::mode_t guest_mode, const x86::cr3_t& guest_cr3, uint64_t base, size_t size);
framework::result<frame_ranges> load_guest_ranges(uint64_t base, size_t size);

}
