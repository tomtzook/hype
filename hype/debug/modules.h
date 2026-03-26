#pragma once

#include <x86/regs.h>
#include <vector.h>
#include <pe.h>

#include "memory/mapping.h"

namespace hype::debug {

struct function_entry {
    function_entry(const pe::section_list& sections, const pe::function_entry& entry);

    const pe::function_entry entry;
    const void* start;
    const void* end;
    const pe::unwind_info unwind_info;
};

class loaded_module {
public:
    loaded_module(framework::buffer&& headers_data, framework::buffer&& functions_data, framework::buffer&& name_data);

    [[nodiscard]] const void* base() const;
    [[nodiscard]] const void* end() const;
    [[nodiscard]] size_t size() const;
    [[nodiscard]] const char* name() const;
    [[nodiscard]] bool contains(const void* ptr) const;

    framework::optional<function_entry> find_function(const void* ptr) const;

private:
    const framework::buffer m_headers_data;
    const framework::buffer m_functions_data;
    const framework::buffer m_name_data;
    const pe::headers m_headers;
};

class loaded_modules {
public:
    loaded_modules() = default;

    const loaded_module* find_module(const void* ptr) const;
    const loaded_module* add_if_image_base(const void* base);

private:
    framework::vector<loaded_module> m_modules;
};

struct stack_frame {
    void* rip;
    void* rbp;
    void* rsp;
};

framework::result<stack_frame> unwind_next(const loaded_module* module, const stack_frame& current);
framework::result<> print_stack_frame(loaded_modules& modules, stack_frame current);
framework::result<> print_stack_frame(loaded_modules& modules, uint64_t rip, uint64_t rbp, uint64_t rsp);

inline __attribute__((always_inline)) void print_current_stack_frame(loaded_modules& modules) {
    print_stack_frame(modules, x86::read_rip(), x86::read_rbp(), x86::read_rsp());
}

}
