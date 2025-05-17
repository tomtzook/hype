
#include <x86/paging/paging.h>
#include <x86/paging/ia32e.h>

#include "base.h"
#include "environment.h"
#include "memory.h"


namespace hype::memory {

#pragma pack(push, 1)

struct allocation_header_t {
    uintptr_t ptr;
    size_t pages;
};

#pragma pack(pop)

void trace_gdt(const x86::segments::gdtr_t& gdtr) {
    auto gdtr_w = x86::segments::table_t(gdtr);
    for (int i = 0; i < gdtr_w.count(); ++i) {
        auto segment = gdtr_w[i];
        TRACE_DEBUG("SEGMENT: i=0x%x, base=0x%llx, limit=0x%llx, s=0x%x, type=0x%x, avail=0x%x, present=0x%x, db=0x%x, dpl=0x%x, long=0x%x, gran=0x%x, raw=0x%llx",
                    i,
                    segment.base_address(), segment.limit(),
                    segment.bits.s,
                    segment.bits.type,
                    segment.bits.available,
                    segment.bits.present,
                    segment.bits.default_db,
                    segment.bits.dpl,
                    segment.bits.long_mode,
                    segment.bits.granularity,
                    segment.raw);
    }
}

status_t setup_initial_guest_gdt() {
    const auto current_gdtr = x86::read<x86::segments::gdtr_t>();

    // make a copy of the current gdt + a tss
    const auto gdt_size = current_gdtr.limit + 1;
    const auto wanted_size = gdt_size + sizeof(x86::segments::descriptor64_t);
    void* new_gdt;
    CHECK(allocate(new_gdt, wanted_size, x86::paging::page_size, memory_type_t::data));

    memset(new_gdt, 0, wanted_size);
    memcpy(new_gdt, reinterpret_cast<const void*>(current_gdtr.base_address), gdt_size);

    // todo: free new_gdt on failure. best be done with raii stuff
    constexpr auto tss_size = sizeof(x86::segments::tss64_t);
    const auto tr_index = gdt_size / sizeof(x86::segments::descriptor_t);
    void* tss;
    CHECK(allocate(tss, tss_size, x86::paging::page_size, memory_type_t::data));

    memset(tss, 0, tss_size);

    auto tss_descriptor = reinterpret_cast<x86::segments::descriptor64_t*>(static_cast<uint8_t*>(new_gdt) + gdt_size);
    tss_descriptor->base_address(environment::to_physical(tss));
    tss_descriptor->limit(tss_size);
    tss_descriptor->base.bits.type = x86::segments::type_t::system_bits32_tss_available;
    tss_descriptor->base.bits.s = x86::segments::descriptor_type_t::system;
    tss_descriptor->base.bits.dpl = 0;
    tss_descriptor->base.bits.present = 1;
    tss_descriptor->base.bits.available = 0;
    tss_descriptor->base.bits.long_mode = 0;
    tss_descriptor->base.bits.default_db = 0;
    tss_descriptor->base.bits.granularity = x86::segments::granularity_t::byte;

    x86::segments::gdtr_t gdtr{};
    gdtr.base_address = reinterpret_cast<uint64_t>(new_gdt);
    gdtr.limit = wanted_size - 1;
    x86::segments::tr_t tr{};
    tr.bits.index = tr_index;
    tr.bits.rpl = 0;
    tr.bits.table = x86::segments::table_type_t::gdt;

    x86::write(gdtr);
    x86::write(tr);

    return {};
}

status_t setup_gdt(x86::segments::gdtr_t& gdtr, gdt_t& gdt, x86::segments::tss64_t& tss) {
    memset(&gdt, 0, sizeof(gdt));
    memset(&tss, 0, sizeof(tss));

    gdtr.base_address = environment::to_physical(&gdt);
    gdtr.limit = sizeof(gdt) - 1;

    gdt.null.raw = 0;

    gdt.code.base_address(0);
    gdt.code.limit(0xfffff);
    gdt.code.bits.type = x86::segments::type_t::code_execute_read;
    gdt.code.bits.s = x86::segments::descriptor_type_t::code_or_data;
    gdt.code.bits.dpl = 0;
    gdt.code.bits.present = 1;
    gdt.code.bits.available = 0;
    gdt.code.bits.long_mode = 1;
    gdt.code.bits.default_db = 0;
    gdt.code.bits.granularity = x86::segments::granularity_t::page;

    gdt.data.base_address(0);
    gdt.data.limit(0xfffff);
    gdt.data.bits.type = x86::segments::type_t::data_read_write;
    gdt.data.bits.s = x86::segments::descriptor_type_t::code_or_data;
    gdt.data.bits.dpl = 0;
    gdt.data.bits.present = 1;
    gdt.data.bits.available = 0;
    gdt.data.bits.long_mode = 0;
    gdt.data.bits.default_db = 1;
    gdt.data.bits.granularity = x86::segments::granularity_t::page;

    gdt.tr.base_address(environment::to_physical(&tss));
    gdt.tr.limit(sizeof(tss));
    gdt.tr.base.bits.type = x86::segments::type_t::system_bits32_tss_available;
    gdt.tr.base.bits.s = x86::segments::descriptor_type_t::system;
    gdt.tr.base.bits.dpl = 0;
    gdt.tr.base.bits.present = 1;
    gdt.tr.base.bits.available = 0;
    gdt.tr.base.bits.long_mode = 0;
    gdt.tr.base.bits.default_db = 0;
    gdt.tr.base.bits.granularity = x86::segments::granularity_t::byte;

    return {};
}

status_t setup_identity_paging(page_table_t& page_table) {
    /*{
        auto& pml4e = page_table.m_pml4[0];
        pml4e.raw = 0;
        pml4e.bits.present = true;
        pml4e.bits.rw = true;
        pml4e.address(environment::to_physical(page_table.m_pdpt));
    }

    for(size_t i = 0; i < x86::paging::ia32e::pdptes_in_pdpt; i++) {
        auto& pdpte = page_table.m_pdpt[i];
        pdpte.raw = 0;
        pdpte.small.present = true;
        pdpte.small.rw = true;
        pdpte.address(environment::to_physical(page_table.m_pd[i]));

        for(size_t j = 0; j < x86::paging::ia32e::pdes_in_directory; j++) {
            auto& pde = page_table.m_pd[i][j];
            pde.raw = 0;
            pde.large.present = true;
            pde.large.rw = true;
            pde.large.ps = true;
            pde.large.pfn = (i * 512) + j;
        }
    }*/

    {
        auto& pml4e = page_table.m_pml4[0];
        pml4e.raw = 0;
        pml4e.bits.present = true;
        pml4e.bits.rw = true;
        pml4e.address(environment::to_physical(page_table.m_pdpt));
    }

    for(size_t i = 0; i < x86::paging::ia32e::pdptes_in_pdpt; i++) {
        auto& pdpte = page_table.m_pdpt[i];
        pdpte.raw = 0;
        pdpte.huge.present = true;
        pdpte.huge.rw = true;
        pdpte.huge.ps = true;
        pdpte.address(i * x86::paging::page_size_1g);
    }

    return {};
}

status_t setup_identity_ept(ept_t& ept, const x86::mtrr::mtrr_cache_t& mtrr_cache) {
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
            pde.large.read = true;
            pde.large.write = true;
            pde.large.execute = true;
            pde.large.ps = true;

            const auto address = (i * 512 + j) * x86::paging::page_size_2m;
            auto type = mtrr_cache.type_for_2m(address);
            CHECK_ASSERT(x86::mtrr::memory_type_invalid != type,
                         "mtrr for range is invalid");

            pde.large.mem_type = static_cast<uint64_t>(type);
            pde.address(address);
        }
    }

    return {};
}

status_t load_page_table(page_table_t& page_table) {
    x86::cr3_t cr3(0);
    cr3.ia32e.address = environment::to_physical(page_table.m_pml4) >> x86::paging::page_bits_4k;
    x86::write(cr3);

    return {};
}

status_t allocate(void*& out, size_t size, size_t alignment, memory_type_t memory_type) {
    void* memory;
    size_t pages = (size + sizeof(allocation_header_t) + alignment - 1) / x86::paging::page_size;
    CHECK(environment::allocate_pages(memory, pages, memory_type));

    void* aligned_mem;
    if (0 != alignment) {
        aligned_mem = reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(memory) +
                sizeof(allocation_header_t)  + alignment - 1) & ~(uintptr_t)(alignment - 1));
    } else {
        aligned_mem = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(memory) + sizeof(allocation_header_t));
    }

    auto header = reinterpret_cast<allocation_header_t*>(reinterpret_cast<uintptr_t>(aligned_mem) - sizeof(allocation_header_t));
    header->ptr = reinterpret_cast<uintptr_t>(memory);
    header->pages = pages;

    out = aligned_mem;
    return {};
}

void free(void* ptr) {
    auto header = reinterpret_cast<allocation_header_t*>(reinterpret_cast<uintptr_t>(ptr) - sizeof(allocation_header_t));
    environment::free_pages(reinterpret_cast<void*>(header->ptr), header->pages);
}

}
