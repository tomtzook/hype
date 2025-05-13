#pragma once

#include "status.h"
#include "memory.h"

namespace hype::environment {

static constexpr size_t max_vcpu_supported = 15;

status_t allocate_pages(void*& out, size_t pages, memory::memory_type_t memory_type);
void free_pages(void* ptr, size_t pages);

physical_address_t to_physical(void* address);
void* to_virtual(physical_address_t address);

size_t get_current_vcpu_id();
void set_current_vcpu_id(size_t id);

status_t get_active_cpu_count(size_t& count);

using vcpu_procedure_t = status_t(void* param);
status_t run_on_all_vcpu(vcpu_procedure_t procedure, void* param);

status_t sleep(size_t microseconds);

}
