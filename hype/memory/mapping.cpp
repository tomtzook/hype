
#include "mapping.h"

#include "environment.h"

namespace hype::memory {

// TODO: NEW MAPPING SCHEME
//  use mapping by size??? 2m vs 4k
//  lock free finding of free spot?? can be done with some structs holding what is free and what is not
//  so no need for processor id to find the pdpte
//  size must be rounded up for page count? store internal info on what's been allocated?
//  maybe also mark which guest it was mapped from??
//  loaded modules will hold the mapping forever (?) by they will be different per guest
//      auto unload of modules if the headers change and do not longer reference a pe?
//      what if guest pages out these pes? maybe act differently for ring 0 and ring 3

guest_memory_mapper::guest_memory_mapper(page_table_t& table, const size_t pml4e_index)
    : m_table(table)
    , m_pml4e_index(pml4e_index)
    , m_mapped() {
    auto& pml4e = m_table.m_pml4[m_pml4e_index];
    pml4e.address(environment::to_physical(m_pdpt));
    pml4e.bits.present = true;
    pml4e.bits.rw = true;
}

framework::result<mapped_memory<guest_memory_mapper>> guest_memory_mapper::map(const uint64_t base, const size_t size) {
    {
        // single page optimization:
        const auto page_start = framework::round_down(base, x86::paging::page_size_4k);
        const auto end_page_start = framework::round_down(base + size, x86::paging::page_size_4k);
        if (page_start == end_page_start) {
            // all the memory is within a single page, no need to fully map it
            const auto translated = environment::to_virtual(verify(gva_to_hpa(base)));
            return framework::ok(mapped_memory<guest_memory_mapper>{nullptr, translated, size});
        }
    }

    const x86::paging::ia32e::linear_address_t guest_address(base);

    const auto ranges = verify(load_guest_ranges(base, size));
    const auto required_pages = count_pages(ranges);

    auto mapped_address = verify(map(required_pages, ranges));
    mapped_address.small.offset = guest_address.small.offset;

    return framework::ok(mapped_memory<guest_memory_mapper>{
        this, reinterpret_cast<void*>(mapped_address.raw), size});
}

framework::result<> guest_memory_mapper::unmap(const void* base, const size_t size) {
    x86::paging::ia32e::linear_address_t address(reinterpret_cast<uint64_t>(base));
    address.small.offset = 0;

    size_t current_pdpte_index = address.small.directory_pointer;
    size_t current_pde_index = address.small.directory;
    size_t current_pte_index = address.small.table;
    for (int i = 0; i < size / x86::paging::page_size_4k; i++) {
        auto& pt = m_pt[current_pdpte_index][current_pde_index];
        auto& pte = pt[current_pte_index];

        pte.raw = 0;

        current_pte_index++;
        if (current_pte_index >= x86::paging::ia32e::ptes_in_table) {
            current_pde_index++;
            current_pte_index = 0;

            if (current_pde_index >= x86::paging::ia32e::pdes_in_directory) {
                current_pdpte_index++;
                current_pde_index = 0;
            }
        }
    }

    {
        framework::unique_lock lock(m_lock);
        remove_range(address);
    }

    return {};
}

framework::result<x86::paging::ia32e::linear_address_t> guest_memory_mapper::map(const size_t required_pages, const frame_ranges& ranges) {
    mapped_range* mapped_range;
    {
        framework::unique_lock lock(m_lock);
        mapped_range = verify(find_and_mark_available_range(required_pages));
    }
    if (mapped_range == nullptr) {
        return framework::err(framework::status_no_space);
    }

    size_t current_pdpte_index = mapped_range->mapped_base.small.directory_pointer;
    size_t current_pde_index = mapped_range->mapped_base.small.directory;
    size_t current_pte_index = mapped_range->mapped_base.small.table;
    for (int i = 0; i < ranges.count; i++) {
        const auto& range = ranges.ranges[i];
        for (int j = 0; j < range.count; j++) {
            const auto frame_number = range.index + j;
            auto& pdpte = m_pdpt[current_pdpte_index];
            auto& pd = m_pd[current_pdpte_index];
            auto& pde = pd[current_pde_index];
            auto& pt = m_pt[current_pdpte_index][current_pde_index];
            auto& pte = pt[current_pte_index];

            if (current_pde_index == 0 && !pdpte.small.present) {
                // since we always map to the same structures, only set this if not present
                pdpte.address(environment::to_physical(pd));
                pdpte.small.present = true;
                pdpte.small.ps = false;
                pdpte.small.rw = true;
            }
            if (current_pte_index == 0 && !pde.small.present) {
                // since we always map to the same structures, only set this if not present
                pde.address(environment::to_physical(pt));
                pde.small.present = true;
                pde.small.ps = false;
                pde.small.rw = true;
            }

            pte.bits.pfn = frame_number;
            pte.bits.present = true;
            pte.bits.rw = true;

            current_pte_index++;
            if (current_pte_index >= x86::paging::ia32e::ptes_in_table) {
                current_pde_index++;
                current_pte_index = 0;

                if (current_pde_index >= x86::paging::ia32e::pdes_in_directory) {
                    current_pdpte_index++;
                    current_pde_index = 0;
                }
            }
        }
    }

    // todo: invalidate tlbs?

    return framework::ok(mapped_range->mapped_base);
}

framework::result<guest_memory_mapper::mapped_range*> guest_memory_mapper::find_and_mark_available_range(const size_t page_count) {
    x86::paging::ia32e::linear_address_t start_address{};
    start_address.small.pml4e = m_pml4e_index;

    for (auto it = m_mapped.begin(); it != m_mapped.end(); ++it) {
        const auto& range = *it;
        if (pages_between(start_address, range.mapped_base) >= page_count) {
            return insert_range_at(it, start_address, page_count);
        }

        start_address.raw = range.mapped_base.raw + (range.mapped_pages * x86::paging::page_size_4k);
    }

    x86::paging::ia32e::linear_address_t end_address{};
    end_address.small.pml4e = m_pml4e_index + 1;
    if (pages_between(start_address, end_address) >= page_count) {
        return insert_range(start_address, page_count);
    }

    return framework::err(framework::status_not_found);
}

framework::result<guest_memory_mapper::mapped_range*> guest_memory_mapper::insert_range_at(
    const framework::vector<mapped_range>::iterator& it, const x86::paging::ia32e::linear_address_t& base, const size_t page_count) {
    auto new_it = verify(m_mapped.insert(it, { base, page_count }));
    return framework::ok(new_it.operator->());
}

framework::result<guest_memory_mapper::mapped_range*> guest_memory_mapper::insert_range(
    const x86::paging::ia32e::linear_address_t& base, const size_t page_count) {
    verify(m_mapped.push_back({ base, page_count }));
    return framework::ok(&m_mapped.back());
}

framework::result<> guest_memory_mapper::remove_range(const x86::paging::ia32e::linear_address_t& base) {
    for (auto it = m_mapped.begin(); it != m_mapped.end(); ++it) {
        if (it->mapped_base.raw == base.raw) {
            verify(m_mapped.erase(it));
            break;
        }
    }

    return {};
}

size_t guest_memory_mapper::count_pages(const frame_ranges& ranges) const {
    size_t count = 0;
    for (int i = 0; i < ranges.count; i++) {
        const auto& range = ranges.ranges[i];
        if (range.count == 0) {
            break;
        }

        count += range.count;
    }

    return count;
}

size_t guest_memory_mapper::pages_between(
    const x86::paging::ia32e::linear_address_t& start, const x86::paging::ia32e::linear_address_t& end) const {
    return (end.raw - start.raw) / x86::paging::page_bits_4k;
}

}
