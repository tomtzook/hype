#pragma once

#include "types.h"
#include "result.h"

namespace framework {

enum class memory_type_t {
    data,
    code
};

namespace environment {

[[nodiscard]] result<void*> allocate_pages(size_t pages, memory_type_t type);
void free_pages(void* ptr, size_t pages, memory_type_t type);

[[nodiscard]] physical_address_t to_physical(const void* address);
[[nodiscard]] void* to_virtual(physical_address_t address);

size_t get_current_vcpu_id();
void set_current_vcpu_id(size_t id);

result<size_t> get_active_cpu_count();

using vcpu_procedure_t = framework::result<>(void* param);
framework::result<> run_on_all_vcpu(vcpu_procedure_t procedure, void* param);

result<> sleep(size_t microseconds);

[[noreturn]] void do_abort();

}
}
