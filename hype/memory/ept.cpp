
#include <pe.h>
#include <environment.h>
#include <base.h>

#include "ept.h"

namespace hype::memory {

static framework::result<framework::span<const uint8_t>> find_code_section(const environment::image_info& image_info) {
    pe::image image(image_info.base, pe::memory_alignment::loaded);
    for (const auto& section : image.sections()) {
        if (0 == memcmp(section.name(), ".text", 4)) {
            return framework::ok(framework::span<const uint8_t>{
                section.rva_to_pointer<uint8_t>(section.virtual_address()),
                section.virtual_size()
            });
        }
    }

    return framework::err(framework::status_not_found);
}

framework::result<> setup_identity_ept(ept_t& ept, const x86::mtrr::mtrr_cache_t& mtrr_cache) {
    size_t current_pt_index = 0;

    {
        auto& pml4e = ept.m_pml4[0];
        pml4e.raw = 0;
        pml4e.bits.read = true;
        pml4e.bits.write = true;
        pml4e.bits.execute = true;
        pml4e.address(environment::to_physical(ept.m_pdpt));
    }

    for (int i = 0; i < x86::vmx::pdptes_in_pdpt; i++) {
        auto& pdpte = ept.m_pdpt[i];
        pdpte.raw = 0;
        pdpte.small.read = true;
        pdpte.small.write = true;
        pdpte.small.execute = true;
        pdpte.address(environment::to_physical(ept.m_pd[i]));

        for (int j = 0; j < x86::vmx::pdes_in_directory; j++) {
            auto& pde = ept.m_pd[i][j];
            pde.raw = 0;

            const uint64_t address = (static_cast<uint64_t>(i) * 512 + j) * x86::paging::page_size_2m;
            const auto type = mtrr_cache.type_for_2m(address);
            assert(x86::mtrr::memory_type_invalid != type, "mtrr for range is invalid");

            if (type == x86::mtrr::memory_type_mixed) {
                assert(current_pt_index < ept_t::pt_count, "not enough pts");
                auto& pt = ept.m_pt[current_pt_index++];

                pde.small.read = true;
                pde.small.write = true;
                pde.small.execute = true;
                pde.address(environment::to_physical(pt));

                for (int k = 0; k < x86::vmx::ptes_in_table; k++) {
                    auto& pte = pt[k];
                    pte.raw = 0;

                    const uint64_t small_address = address + (static_cast<uint64_t>(k) * x86::paging::page_size);
                    const auto small_type = mtrr_cache.type_for_4k(small_address);
                    assert(x86::mtrr::memory_type_invalid != small_type, "mtrr for range is invalid");

                    pte.bits.read = true;
                    pte.bits.write = true;
                    pte.bits.execute = true;
                    pte.bits.mem_type = static_cast<uint64_t>(small_type);
                    pte.address(small_address);
                }

            } else {
                pde.large.read = true;
                pde.large.write = true;
                pde.large.execute = true;
                pde.large.ps = true;

                pde.large.mem_type = static_cast<uint64_t>(type);
                pde.address(address);
            }
        }
    }

    return {};
}

framework::result<> split_large_page_to_small_pages(ept_t& ept, const uint32_t pml4e_index, const uint32_t pdpte_index, const uint32_t pde_index, x86::paging::to_virtual to_virtual) {
    auto pml4 = ept.m_pml4;
    auto& pml4e = pml4[pml4e_index];
    if (!pml4e.present()) {
        return framework::err(status_pml4e_not_present);
    }

    const auto pdpt_address = pml4e.address();
    auto* pdpt = static_cast<x86::vmx::pdpte_t*>(to_virtual != nullptr ? to_virtual(pdpt_address) : reinterpret_cast<void*>(pdpt_address));
    auto& pdpte = pdpt[pdpte_index];
    if (!pdpte.present()) {
        return framework::err(status_pdpte_not_present);
    }
    if (pdpte.is_huge()) {
        return framework::err(status_pdpte_is_huge);
    }

    const auto pd_address = pdpte.address();
    auto* pd = static_cast<x86::vmx::pde_t*>(to_virtual != nullptr ? to_virtual(pd_address) : reinterpret_cast<void*>(pd_address));
    auto& pde = pd[pde_index];
    if (!pde.present()) {
        return framework::err(status_pde_not_present);
    }
    if (!pde.is_large()) {
        return framework::err(status_pde_is_small);
    }

    const auto read = pde.large.read;
    const auto write = pde.large.write;
    const auto execute = pde.large.execute;
    const auto type = pde.large.mem_type;
    const auto start_address = pde.address();

    struct table_t {
        page_aligned x86::vmx::pte_t entries[x86::vmx::ptes_in_table];
    };
    // ReSharper disable once CppDFAMemoryLeak
    // this leak is intentional
    auto* table = new table_t;
    verify_alloc(table);

    auto& pt = table->entries;
    pde.raw = 0;
    pde.small.read = read;
    pde.small.write = write;
    pde.small.execute = execute;
    pde.address(environment::to_physical(pt));

    for (int i = 0; i < x86::vmx::ptes_in_table; i++) {
        auto& pte = pt[i];
        pte.raw = 0;
        pte.bits.read = read;
        pte.bits.write = write;
        pte.bits.execute = execute;
        pte.bits.mem_type = type;

        const auto address = start_address + (static_cast<uint64_t>(i) * x86::paging::page_bits_4k);
        pte.address(address);
    }

    return {};
}

using namespace x86::vmx;
using namespace x86;

bool apply_permissions1(const ept_pointer_t& eptp, const guest_physical_address_t address, const bool read, const bool write, const bool execute, paging::to_virtual to_virtual) {
    const auto pml4_address = static_cast<physical_address_t>(eptp.bits.address) << paging::page_bits_4k;
    auto pml4 = reinterpret_cast<pml4e_t*>(pml4_address);
    auto& pml4e = pml4[address.huge.pml4e];
    if (!pml4e.present()) {
        return false;
    }

    if (read) pml4e.bits.read = true;
    if (write) pml4e.bits.write = true;
    if (execute) pml4e.bits.execute = true;

    const auto pdpt_address = pml4e.address();
    auto* pdpt = static_cast<pdpte_t*>(to_virtual != nullptr ? to_virtual(pdpt_address) : reinterpret_cast<void*>(pdpt_address));
    auto& pdpte = pdpt[address.huge.directory_pointer];
    if (!pdpte.present()) {
        return false;
    }

    if (pdpte.is_huge()) {
        pdpte.huge.read = read;
        pdpte.huge.write = write;
        pdpte.huge.execute = execute;
        return true;
    }

    if (read) pdpte.small.read = true;
    if (write) pdpte.small.write = true;
    if (execute) pdpte.small.execute = true;

    const auto pd_address = pdpte.address();
    auto* pd = static_cast<pde_t*>(to_virtual != nullptr ? to_virtual(pd_address) : reinterpret_cast<void*>(pd_address));
    auto& pde = pd[address.large.directory];
    if (!pde.present()) {
        return false;
    }

    if (pde.is_large()) {
        pde.large.read = read;
        pde.large.write = write;
        pde.large.execute = execute;
        return true;
    }

    if (read) pde.small.read = true;
    if (write) pde.small.write = true;
    if (execute) pde.small.execute = true;

    const auto pt_address = pde.address();
    auto* pt = static_cast<pte_t*>(to_virtual != nullptr ? to_virtual(pt_address) : reinterpret_cast<void*>(pt_address));
    auto& pte = pt[address.small.table];
    if (!pte.present()) {
        return false;
    }

    pte.bits.read = read;
    pte.bits.write = write;
    pte.bits.execute = execute;
    return true;
}

void apply_permissions1(const ept_pointer_t& eptp, const physical_address_t start_address, const size_t size, const bool read, const bool write, const bool execute, const paging::to_virtual to_virtual) {
    // todo: optimize based on paging structs and mapping size
    const guest_physical_address_t start = start_address & ~((1 << paging::page_bits_4k) - 1);
    const guest_physical_address_t end = (start_address + size) & ~((1 << paging::page_bits_4k) - 1);

    for (auto address = start; address.raw <= end.raw; address.raw += paging::page_size_4k) {
        //trace_debug("setting permissions at 0x%llx", address);
        apply_permissions1(eptp, address, read, write, execute, to_virtual);
    }
}

framework::result<> protect_image(ept_t& ept, const environment::image_info& image_info) {
    x86::vmx::ept_pointer_t eptp{};
    eptp.bits.mem_type = x86::mtrr::memory_type_t::writeback;
    eptp.bits.walk_length = 3;
    eptp.address(environment::to_physical(&ept.m_pml4));

    /*pe::image image(image_info.base, pe::memory_alignment::loaded);
    for (const auto& section : image.sections()) {
        const auto base = section.rva_to_pointer<uint8_t>(section.virtual_address());
        const auto size = section.virtual_size();
        trace_debug("section %a 0x%llx 0x%x", section.name(), base, size);

        const guest_physical_address_t start = environment::to_physical(base) & ~((1 << paging::page_bits_4k) - 1);
        split_large_page_to_small_pages(ept, start.large.pml4e, start.large.directory_pointer, start.large.directory, nullptr);
        apply_permissions1(eptp, environment::to_physical(base), size, true, false, true, nullptr);
    }*/

    const guest_physical_address_t start = environment::to_physical(image_info.base) & ~((1 << paging::page_bits_4k) - 1);
    split_large_page_to_small_pages(ept, start.large.pml4e, start.large.directory_pointer, start.large.directory, nullptr);
    apply_permissions1(eptp, environment::to_physical(image_info.base), image_info.size, true, false, true, nullptr);

    //const auto code_section = verify(find_code_section(image_info));
    //trace_debug("code section 0x%llx 0x%x", code_section.data(), code_section.size());

    return {};
}

}
