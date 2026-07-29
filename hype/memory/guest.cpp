
#include <x86/paging/ia32e.h>
#include <x86/vmx/vmcs.h>
#include <math.h>

#include "guest.h"

namespace hype::memory {

template<typename t_>
framework::result<const t_*> get_guest_page_table_entry(const x86::vmx::ept_pointer_t& eptp, const uint64_t address) {
    const auto translated = verify(gpa_to_hpa(eptp, address));
    return framework::ok(reinterpret_cast<const t_*>(translated));
}

framework::result<x86::vmx::ept_pointer_t> read_current_eptp() {
    x86::vmx::ept_pointer_t eptp;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_ept_pointer, eptp.raw));

    return framework::ok(eptp);
}

framework::result<x86::cr3_t> read_current_guest_cr3() {
    x86::cr3_t cr3;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_cr3, cr3.raw));

    return framework::ok(cr3);
}

framework::result<x86::paging::mode_t> read_current_guest_paging_mode() {
    x86::cr0_t cr0;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_cr0, cr0.raw));
    x86::cr4_t cr4;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_cr4, cr4.raw));
    x86::msr::ia32_efer_t efer;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_efer, efer.raw));

    x86::paging::mode_t mode;
    if (cr0.bits.paging_enable && cr4.bits.physical_address_extension) {
        if (efer.bits.lma) {
            mode = x86::paging::mode_t::ia32e;
        } else {
            mode = x86::paging::mode_t::pae;
        }
    } else if (cr0.bits.paging_enable) {
        mode = x86::paging::mode_t::bit32;
    } else {
        mode = x86::paging::mode_t::disabled;
    }

    return framework::ok(mode);
}

framework::result<physical_address_t> gpa_to_hpa(const x86::vmx::ept_pointer_t& eptp, const uint64_t address) {
    physical_address_t result;
    if (x86::vmx::to_physical(eptp, address, result)) {
        return framework::ok(result);
    }

    return framework::err(framework::status_not_found);
}

framework::result<physical_address_t> gva_to_gpa_ia32(const x86::vmx::ept_pointer_t& eptp, const x86::cr3_t& guest_cr3, const uint64_t address) {
    const x86::paging::ia32e::linear_address_t linear_address(address);

    const auto pml4_address = static_cast<physical_address_t>(guest_cr3.ia32e.address) << x86::paging::page_bits_4k;
    const auto* pml4 = verify(get_guest_page_table_entry<x86::paging::ia32e::pml4e_t>(eptp, pml4_address));
    const auto& pml4e = pml4[linear_address.huge.pml4e];
    if (!pml4e.bits.present) {
        return framework::err(framework::status_not_found);
    }

    const auto* pdpt = verify(get_guest_page_table_entry<x86::paging::ia32e::pdpte_t>(eptp, pml4e.address()));
    const auto& pdpte = pdpt[linear_address.huge.directory_pointer];
    if (!pdpte.huge.present) {
        return framework::err(framework::status_not_found);
    }
    if (pdpte.is_huge()) {
        return framework::ok(pdpte.address() | static_cast<physical_address_t>(linear_address.huge.offset));
    }

    const auto* pd = verify(get_guest_page_table_entry<x86::paging::ia32e::pde_t>(eptp, pdpte.address()));
    const auto& pde = pd[linear_address.large.directory];
    if (!pde.large.present) {
        return framework::err(framework::status_not_found);
    }
    if (pde.is_large()) {
        return framework::ok(pde.address() | static_cast<physical_address_t>(linear_address.large.offset));
    }

    const auto* pt = verify(get_guest_page_table_entry<x86::paging::ia32e::pte_t>(eptp, pde.address()));
    const auto& pte = pt[linear_address.small.table];
    if (!pte.bits.present) {
        return framework::err(framework::status_not_found);
    }

    return framework::ok(pte.address() | static_cast<physical_address_t>(linear_address.small.offset));
}

framework::result<physical_address_t> gva_to_gpa(const x86::vmx::ept_pointer_t& eptp, const x86::paging::mode_t guest_mode, const x86::cr3_t& guest_cr3, const uint64_t address) {
    switch (guest_mode) {
        case x86::paging::mode_t::ia32e:
            return gva_to_gpa_ia32(eptp, guest_cr3, address);
        default:
            return framework::err(framework::status_unimplemented);
    }
}

framework::result<physical_address_t> gva_to_hpa(const x86::vmx::ept_pointer_t& eptp, const x86::paging::mode_t guest_mode, const x86::cr3_t& guest_cr3, const uint64_t address) {
    const auto gpa = verify(gva_to_gpa(eptp, guest_mode, guest_cr3, address));
    return gpa_to_hpa(eptp, gpa);
}

framework::result<physical_address_t> gva_to_hpa(const uint64_t address) {
    const auto eptp = verify(read_current_eptp());
    const auto guest_cr3 = verify(read_current_guest_cr3());
    const auto guest_paging_mode = verify(read_current_guest_paging_mode());

    return gva_to_hpa(eptp, guest_paging_mode, guest_cr3, address);
}

framework::result<frame_ranges> load_guest_ranges(const x86::vmx::ept_pointer_t& eptp, const x86::paging::mode_t guest_mode, const x86::cr3_t& guest_cr3, const uint64_t base, const size_t size) {
    frame_ranges ranges{};
    for (auto address = framework::round_down(base, x86::paging::page_size_4k); address < base + size; address += x86::paging::page_size_4k) {
        const auto physical_addr = verify(gva_to_hpa(eptp, guest_mode, guest_cr3, address));
        const auto pfn = physical_addr >> x86::paging::page_bits_4k;

        auto& range = ranges.ranges[ranges.count];
        if (range.count == 0) {
            range.index = pfn;
            range.count = 1;
            ranges.count++;
        } else if (range.index + 1 == pfn) {
            range.count++;
        } else {
            auto& next_range = ranges.ranges[++ranges.count];
            next_range.index = pfn;
            next_range.count = 1;
        }
    }

    return framework::ok(ranges);
}

framework::result<frame_ranges> load_guest_ranges(const uint64_t base, const size_t size) {
    const auto eptp = verify(read_current_eptp());
    const auto guest_cr3 = verify(read_current_guest_cr3());
    const auto guest_paging_mode = verify(read_current_guest_paging_mode());

    return load_guest_ranges(eptp, guest_paging_mode, guest_cr3, base, size);
}

}
