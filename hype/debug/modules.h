#pragma once

#include <x86/regs.h>
#include <vector.h>
#include <pe.h>

namespace hype::debug {

struct function_entry {
    function_entry(const pe::section_list& sections, const pe::function_entry& entry);

    const void* start;
    const void* end;
    const pe::unwind_info unwind_info;
};

class loaded_module {
public:
    explicit loaded_module(const void* image_base);

    [[nodiscard]] const void* base() const;
    [[nodiscard]] const void* end() const;
    [[nodiscard]] size_t size() const;
    [[nodiscard]] const char* name() const;
    [[nodiscard]] bool contains(const void* ptr) const;

    framework::optional<function_entry> find_function(const void* ptr) const;

private:
    const pe::image m_image;
    const void* m_start;
    const void* m_end;
    const pe::functions_table m_function_table;
};

class loaded_modules {
public:
    loaded_modules() = default;

    const loaded_module* find_module(const void* ptr) const;

    void add_if_image_base(const void* base);

private:
    framework::vector<loaded_module> m_modules;
};

struct stack_frame {
    const loaded_module* module;
    void* rip;
    void* rbp;
};

framework::result<stack_frame> unwind_next(const loaded_modules& modules, stack_frame current);
framework::result<> print_stack_frame(const loaded_modules& modules, stack_frame current);
framework::result<> print_stack_frame(const loaded_modules& modules, uint64_t rip, uint64_t rbp);

inline __attribute__((always_inline)) void print_current_stack_frame(const loaded_modules& modules) {
    print_stack_frame(modules, x86::read_rip(), x86::read_rbp());
}

}
