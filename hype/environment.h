#pragma once

#include "status.h"
#include "memory.h"

namespace hype::environment {

static constexpr size_t max_vcpu_supported = 15;

status_t allocate_pages(void*& out, size_t pages, memory::memory_type_t heap_type) noexcept;
void free_pages(void* ptr, size_t pages) noexcept;

physical_address_t to_physical(void* address) noexcept;
void* to_virtual(physical_address_t address) noexcept;

size_t get_current_vcpu_id() noexcept;
void set_current_vcpu_id(size_t id) noexcept;

status_t get_active_cpu_count(size_t& count) noexcept;

using vcpu_procedure_t = status_t(void* param) noexcept;
status_t run_on_all_vcpu(vcpu_procedure_t procedure, void* param) noexcept;

}
