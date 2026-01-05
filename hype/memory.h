#pragma once

#include <x86/paging/ia32e.h>
#include <x86/mtrr.h>
#include <x86/vmx/ept.h>
#include <x86/segments.h>

#include <environment.h>
#include <math.h>
#include <base.h>

namespace hype::memory {
#pragma pack(push, 1)

struct gdt_t {
    static constexpr size_t code_descriptor_index = 1;
    static constexpr size_t data_descriptor_index = 2;
    static constexpr size_t tss_descriptor_index = 3;

    x86::segments::descriptor_t null;
    x86::segments::descriptor_t code;
    x86::segments::descriptor_t data;
    x86::segments::descriptor64_t tr;
};

struct page_table_t {
    static constexpr size_t stack_guard_pml4e = 5;

    page_aligned x86::paging::ia32e::pml4e_t m_pml4[x86::paging::ia32e::pml4e_in_pml4];
    page_aligned x86::paging::ia32e::pdpte_t m_pdpt[x86::paging::ia32e::pdptes_in_pdpt];
    page_aligned x86::paging::ia32e::pde_t m_pd[x86::paging::ia32e::pdptes_in_pdpt][x86::paging::ia32e::pdes_in_directory];
};

struct ept_t {
    page_aligned x86::vmx::pml4e_t m_pml4[x86::vmx::pml4e_in_pml4];
    page_aligned x86::vmx::pdpte_t m_pdpt[x86::vmx::pdptes_in_pdpt];
    page_aligned x86::vmx::pde_t m_pd[x86::vmx::pdptes_in_pdpt][x86::vmx::pdes_in_directory];
};

#pragma pack(pop)

template<size_t size_>
struct stack {
    static constexpr size_t stack_size = framework::round_up(size_, x86::paging::page_size);

    stack() = default;
    stack(const stack&) = default;
    stack(stack&&) = default;
    stack& operator=(const stack&) = default;
    stack& operator=(stack&&) = default;

    [[nodiscard]] uint8_t* start() const { return m_ptr; }
    [[nodiscard]] uint8_t* end() const { return m_ptr + stack_size; }
    [[nodiscard]] constexpr size_t size() const { return stack_size; }

    void remap_to(const linear_address_t new_base_address) { m_ptr = reinterpret_cast<uint8_t*>(new_base_address); }

private:
    page_aligned uint8_t m_data[stack_size]{};
    uint8_t* m_ptr = m_data;
};

struct stack_guard {
    stack_guard() = default;
    stack_guard(const stack_guard&) = default;
    stack_guard(stack_guard&&) = default;
    stack_guard& operator=(const stack_guard&) = default;
    stack_guard& operator=(stack_guard&&) = default;

    void map_into_pml4e(page_table_t& table, size_t pml4e_index);

    template<size_t size_>
    void create_guard(stack<size_>& target);

private:
    void map_into_pml4e(x86::paging::ia32e::pml4e_t& pml4e);

    // we'll use a single pdpt, but need all to map to a pml4e
    page_aligned x86::paging::ia32e::pdpte_t m_pdpt[x86::paging::ia32e::pdptes_in_pdpt]{};
    page_aligned x86::paging::ia32e::pde_t m_pd[x86::paging::ia32e::pdes_in_directory]{};
    page_aligned x86::paging::ia32e::pte_t m_pt[x86::paging::ia32e::pdes_in_directory][x86::paging::ia32e::ptes_in_table]{};

    size_t m_pml4e_index = 0;
    volatile uint16_t m_pde_index{};
};

void trace_gdt(const x86::segments::gdtr_t& gdtr);

// also loads the gdt
framework::result<> setup_initial_guest_gdt();

framework::result<> setup_gdt(x86::segments::gdtr_t& gdtr, gdt_t& gdt, x86::segments::tss64_t& tss);
framework::result<> setup_identity_paging(page_table_t& page_table);
framework::result<> setup_identity_ept(ept_t& ept, const x86::mtrr::mtrr_cache_t& mtrr_cache);

framework::result<> load_page_table(const page_table_t& page_table);

template<size_t size_>
void stack_guard::create_guard(stack<size_>& target) {
    static_assert(stack<size_>::stack_size <= x86::paging::page_size_1m, "stack to big to be mapped into one pde");

    // get 3 pdes per guard
    const auto first_pde_index = x86::atomic::fetchadd16(&m_pde_index, 3);

    // first and last pde need to be cleared (non-present)
    auto& first_pde = m_pd[first_pde_index];
    first_pde.raw = 0;
    auto& last_pde = m_pd[first_pde_index + 2];
    last_pde.raw = 0;

    const auto stack_pde_index = first_pde_index + 1;
    auto& stack_pde = m_pd[stack_pde_index];
    auto& stack_pt = m_pt[stack_pde_index];

    stack_pde.raw = 0;
    stack_pde.small.rw = true;
    stack_pde.small.present = true;
    stack_pde.address(environment::to_physical(stack_pt));

    constexpr auto stack_pages = stack<size_>::stack_size / x86::paging::page_size;
    const auto stack_start_address = target.start();
    for (auto i = 0; i < stack_pages; ++i) {
        const auto stack_address = stack_start_address + i * x86::paging::page_size;
        const auto physical_address = environment::to_physical(stack_address);

        auto& pte = stack_pt[i];
        pte.raw = 0;
        pte.bits.rw = true;
        pte.bits.present = true;
        pte.address(physical_address);
    }

    x86::paging::ia32e::linear_address_t linear_address{};
    linear_address.small.pml4e = m_pml4e_index;
    linear_address.small.directory_pointer = 0;
    linear_address.small.directory = stack_pde_index;
    linear_address.small.table = 0;
    linear_address.small.offset = 0;

    target.remap_to(linear_address.raw);
}

}