
#include <base.h>

#include "print.h"
#include "modules.h"

namespace hype::debug {

static const char* find_image_name(const pe::image& image) {
    const auto export_table = image.load_export_table();
    if (export_table.is_valid()) {
        return export_table.image_name();
    }

    return "";
}

static framework::result<size_t> find_setfp_code_index(const pe::unwind_info& unwind_info) {
    for (size_t i = 0; i < unwind_info.codes_count(); i++) {
        const auto code = unwind_info.code(i);

        const auto opcode = static_cast<pe::UnwindCodeOpCode>(code->u.OpCode);
        switch (opcode) {;
            case pe::uwop_alloc_large:
                if (code->u.OpInfo == 0) {
                    i += 1;
                } else {
                    i += 2;
                }
                break;
            case pe::uwop_alloc_small:
            case pe::uwop_push_nonvol:
            case pe::uwop_save_nonvol:
            case pe::uwop_save_nonvol_far:
            case pe::uwop_save_xmm128:
            case pe::uwop_save_xmm128_far:
            case pe::uwop_epilog:
            case pe::uwop_spare_code:
            case pe::uwop_push_machframe:
                break;
            case pe::uwop_set_fpreg:
                return framework::ok(i);
        }
    }

    return framework::err(framework::status_bad_arg); //todo: not found
}

static framework::result<size_t> calc_prolog_rsp_offset(const pe::unwind_info& unwind_info) {
    const auto start_idx = verify(find_setfp_code_index(unwind_info)) + 1;

    size_t offset = 0;
    for (auto i = start_idx; i < unwind_info.codes_count(); i++) {
        const auto code = unwind_info.code(i);
        const auto opcode = static_cast<pe::UnwindCodeOpCode>(code->u.OpCode);
        switch (opcode) {;
            case pe::uwop_alloc_large: {
                size_t alloc_size;
                if (code->u.OpInfo == 0) {
                    alloc_size = unwind_info.code(i + 1)->FrameOffset * 8;
                    i += 1;
                } else {
                    alloc_size = *reinterpret_cast<const unsigned int*>(unwind_info.code(i + 1));
                    i += 2;
                }

                offset += alloc_size;
                break;
            }
            case pe::uwop_alloc_small: {
                const auto alloc_size = (code->u.OpInfo * 8) + 8;
                offset += alloc_size;
                break;
            }
            case pe::uwop_push_nonvol:
            case pe::uwop_save_nonvol:
            case pe::uwop_save_nonvol_far:
                offset += 8;
                break;
            case pe::uwop_save_xmm128:
            case pe::uwop_save_xmm128_far:
                // todo: handle
                break;
            case pe::uwop_epilog:
            case pe::uwop_spare_code:
            case pe::uwop_push_machframe:
                // todo: handle
                break;
            case pe::uwop_set_fpreg:
                break;
        }
    }

    return framework::ok(offset);
}

function_entry::function_entry(const pe::section_list& sections, const pe::function_entry& entry)
    : start(sections.rva_to_pointer<void>(entry.start()))
    , end(sections.rva_to_pointer<void>(entry.end()))
    , unwind_info(entry.find_unwind_info(sections))
{}

loaded_module::loaded_module(const void* image_base)
    : m_image(image_base, pe::memory_alignment::loaded)
    , m_start(m_image.base())
    , m_end(m_image.end())
    , m_function_table(m_image.load_exception_table())
{}

const void* loaded_module::base() const {
    return m_start;
}

const void* loaded_module::end() const {
    return m_end;
}

size_t loaded_module::size() const {
    return m_image.size();
}

const char* loaded_module::name() const {
    return find_image_name(m_image);
}

bool loaded_module::contains(const void* ptr) const {
    return ptr >= m_start && ptr < m_end;
}

framework::optional<function_entry> loaded_module::find_function(const void* ptr) const {
    if (!m_function_table.is_valid()) {
        return framework::nullopt;
    }

    // todo: binary search
    const auto offset = static_cast<const uint8_t*>(ptr) - m_image.base();
    const auto sections = m_image.sections();
    for (const auto& entry : m_function_table) {
        const auto start = sections.rva_to_offset(entry.start());
        const auto end = sections.rva_to_offset(entry.end());
        if (offset >= start && offset < end) {
            return function_entry(sections, entry);
        }
    }

    return framework::nullopt;
}

const loaded_module* loaded_modules::find_module(const void* ptr) const {
    // todo: binary search
    for (const auto& module : m_modules) {
        if (module.contains(ptr)) {
            return &module;
        }
    }

    return nullptr;
}

void loaded_modules::add_if_image_base(const void* base) {
    if (pe::image::is_image_base(base)) {
        m_modules.push_back(loaded_module(base));
    }
}

framework::result<stack_frame> unwind_next(const loaded_modules& modules, const stack_frame current) {
    const auto* module = modules.find_module(current.rip);
    if (module == nullptr) {
        // trace_debug("did not find module for 0x%p", current.rip);
        return framework::err(framework::status_not_found);
    }

    const auto function_opt = module->find_function(current.rip);
    if (!function_opt) {
        // trace_debug("did not find function for 0x%p in module 0x%p", current.rip, module->base());
        return framework::err(framework::status_not_found);
    }

    const auto unwind_info = function_opt.value().unwind_info;
    const auto unwind_fr = unwind_info.frame_register();
    if (unwind_fr == 0 || static_cast<pe::UnwindCodeOpInfo>(unwind_fr) != pe::uwinfo_rbp) {
        return framework::err(framework::status_unsupported);
    }

    // assuming that the function starts with push rbp
    const auto fp = reinterpret_cast<uint64_t>(current.rbp);
    const auto offset = unwind_info.frame_register_offset() * 16;
    const auto prolog_offset = verify(calc_prolog_rsp_offset(unwind_info));
    const auto last_rbp = *reinterpret_cast<uint64_t*>(fp + prolog_offset - offset - 8);
    const auto return_address = *reinterpret_cast<uint64_t*>(fp + prolog_offset - offset);
    return framework::ok(stack_frame{
        module,
        reinterpret_cast<void*>(return_address),
        reinterpret_cast<void*>(last_rbp)});
}

framework::result<> print_stack_frame(const loaded_modules& modules, stack_frame current) {
    char buffer[512];
    size_t offset = 0;
    size_t buffer_size = sizeof(buffer);

    ascii_format(buffer, offset, buffer_size, "Stack Unwind:\n");
    ascii_format(buffer, offset, buffer_size, "\tFrame: [Module %a (0x%p -> 0x%p)] rip=0x%p, rbp=0x%p\n",
        current.module->name(), current.module->base(), current.module->end(),
        current.rip, current.rbp);
    do {
        const auto result = unwind_next(modules, current);
        if (!result) {
            break;
        }

        current = result.value();
        ascii_format(buffer, offset, buffer_size, "\tFrame: [Module %a (0x%p -> 0x%p)] rip=0x%p, rbp=0x%p\n",
            current.module->name(), current.module->base(), current.module->end(),
            current.rip, current.rbp);
    } while (true);

    buffer[offset] = '\0';
    trace_debug("%a", buffer);

    return {};
}

framework::result<> print_stack_frame(const loaded_modules& modules, const uint64_t rip, const uint64_t rbp) {
    const auto* module = modules.find_module(reinterpret_cast<void*>(rip));
    if (module == nullptr) {
        return framework::err(framework::status_not_found);
    }

    return print_stack_frame(modules, stack_frame{module, reinterpret_cast<void*>(rip), reinterpret_cast<void*>(rbp)});
}

}
