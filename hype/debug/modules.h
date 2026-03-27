#pragma once

#include <x86/regs.h>
#include <vector.h>
#include "str.h"
#include <pe.h>

#include "memory/mapping.h"

namespace hype::debug {

// NOTICE: WRITTEN WITH GUEST MAPPING AT HEART, so everything is translated from guest

struct function_entry {
    using mem_t_ = memory::guest_memory_mapper;

    function_entry(const void* guest_image_base, const pe::ImageRuntimeFunctionEntry& function_entry, memory::mapped_memory<mem_t_>&& unwind_info_map);

    const void* guest_start;
    const void* guest_end;
private:
    memory::mapped_memory<mem_t_> unwind_info_map;
public:
    const pe::unwind_info unwind_info;
};

class loaded_module {
public:
    using mem_t_ = memory::guest_memory_mapper;

    loaded_module(memory::memory_mapper<mem_t_> mapper, const void* guest_image_base, memory::mapped_memory<mem_t_>&& headers_data, memory::mapped_memory<mem_t_>&& functions_data, framework::optional<framework::string>&& name);

    [[nodiscard]] const void* guest_base() const;
    [[nodiscard]] const void* guest_end() const;
    [[nodiscard]] size_t size() const;
    [[nodiscard]] const char* name() const;
    [[nodiscard]] bool contains(const void* guest_ptr) const;

    framework::result<function_entry> find_function(const void* guest_ptr) const;

private:
    memory::memory_mapper<mem_t_> m_mapper;
    const void* m_guest_image_base;
    memory::mapped_memory<mem_t_> m_headers_data;
    memory::mapped_memory<mem_t_> m_functions_data;
    framework::optional<framework::string> m_name;
    pe::headers m_headers;
};

class loaded_modules {
public:
    using mem_t_ = memory::guest_memory_mapper;

    explicit loaded_modules(memory::memory_mapper<mem_t_> mapper);

    framework::result<const loaded_module&> find_module(const void* ptr) const;
    framework::result<const loaded_module&> add(const void* base);

private:
    memory::memory_mapper<mem_t_> m_mapper;
    framework::vector<loaded_module> m_modules;
};

struct stack_frame {
    void* rip;
    void* rbp;
    void* rsp;
};

framework::result<stack_frame> unwind_next(memory::memory_mapper<memory::guest_memory_mapper>& mapper, const loaded_module& module, const stack_frame& current);
framework::result<> print_stack_frame(memory::memory_mapper<memory::guest_memory_mapper>& mapper, loaded_modules& modules, stack_frame current);
framework::result<> print_stack_frame(memory::memory_mapper<memory::guest_memory_mapper>& mapper, loaded_modules& modules, uint64_t rip, uint64_t rbp, uint64_t rsp);

/*
inline __attribute__((always_inline)) void print_current_stack_frame(loaded_modules& modules) {
    print_stack_frame(modules, x86::read_rip(), x86::read_rbp(), x86::read_rsp());
}
*/
}
