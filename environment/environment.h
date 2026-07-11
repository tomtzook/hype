#pragma once

#include "base.h"

namespace environment {

struct image_info {
    void* base;
    size_t size;
};

framework::result<> initialize();

framework::result<image_info> get_our_image_info();

[[nodiscard]] framework::result<void*> allocate_pages(size_t pages, framework::memory_type type);
void free_pages(void* ptr, size_t pages, framework::memory_type type);

[[nodiscard]] physical_address_t to_physical(const void* address);
[[nodiscard]] void* to_virtual(physical_address_t address);

size_t get_current_vcpu_id();
void set_current_vcpu_id(size_t id);

framework::result<size_t> get_active_cpu_count();

using vcpu_procedure_t = framework::result<>(void* param);
framework::result<> run_on_all_vcpu(vcpu_procedure_t procedure, void* param);

framework::result<> sleep(size_t microseconds);

framework::result<> serial_initialize();
framework::result<bool> serial_available();
framework::result<char> serial_read();
framework::result<bool> serial2_available();
framework::result<char> serial2_read();
framework::result<> serial2_write(char ch);

}
