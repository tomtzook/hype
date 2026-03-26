
#include <base.h>

#include "print.h"
#include "modules.h"

#include "context.h"

namespace hype::debug {

static const char* pdb_path_to_image_name(const char* name) {
    return framework::strrchr(name, '/') + 1;
}

static const char* find_image_name(const pe::image& image) {
    const auto export_table = image.load_export_table();
    if (export_table.is_valid()) {
        return export_table.image_name();
    }

    const auto debug_table = image.load_debug_table();
    if (debug_table.is_valid() && debug_table.type() == pe::image_debug_type_codeview) {
        const auto path = debug_table.data<pe::CvInfoPdb70>(image.sections())->PdbFileName;
        return pdb_path_to_image_name(path);
    }

    return "n/a";
}

static framework::result<uint64_t> get_frame_register(const pe::unwind_info& unwind_info, const stack_frame& frame) {
    const auto fp = static_cast<pe::UnwindCodeOpInfo>(unwind_info.frame_register());
    if (fp == 0) {
        return framework::err(framework::status_not_found);
    }

    switch (fp) {
        case pe::uwinfo_rbp:
            return framework::ok(reinterpret_cast<uint64_t>(frame.rbp));
        default:
            return framework::err(framework::status_unsupported);
    }
}

static framework::result<void*> calculate_original_rsp(const pe::unwind_info& unwind_info, const stack_frame& frame) {
    auto rsp = reinterpret_cast<uint64_t>(frame.rsp);
    for (auto i = 0; i < unwind_info.codes_count(); i++) {
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

                rsp += alloc_size;
                break;
            }
            case pe::uwop_alloc_small: {
                const auto alloc_size = (code->u.OpInfo * 8) + 8;
                rsp += alloc_size;
                break;
            }
            case pe::uwop_push_nonvol:
                rsp += 8;
                break;
            case pe::uwop_save_nonvol:
                // saves register to stack, no rsp change
                i += 1;
                break;
            case pe::uwop_save_nonvol_far:
                // saves register to stack, no rsp change
                i += 2;
                break;
            case pe::uwop_save_xmm128:
                // saves register to stack, no rsp change
                i += 1;
                break;
            case pe::uwop_save_xmm128_far:
                // saves register to stack, no rsp change
                i += 2;
                break;
            case pe::uwop_set_fpreg: {
                // set the value of the fpreg, into some stack value
                const auto fp = verify(get_frame_register(unwind_info, frame));
                const auto offset = unwind_info.frame_register_offset() * 16;
                rsp = fp - offset;
                break;
            }
            case pe::uwop_push_machframe: {
                switch (code->u.OpInfo) {
                    case 0:
                        rsp += 40;
                        break;
                    case 1:
                        rsp += 48;
                        break;
                    default:
                        return framework::err(framework::status_unsupported);
                }
                break;
            }
            case pe::uwop_epilog:
            case pe::uwop_spare_code:
                break;
            default:
                return framework::err(framework::status_unsupported);
        }
    }

    return framework::ok(reinterpret_cast<void*>(rsp));
}

template<memory::memory_mapper_type mem_t_>
framework::result<framework::buffer> load_module_headers(memory::memory_mapper<mem_t_>& memory_mapper, const void* image_base) {
    const auto first_page = verify(memory_mapper->map(reinterpret_cast<uint64_t>(image_base), x86::paging::page_size_4k));
    const auto* dos_header = first_page.template data<pe::ImageDosHeader>();
    if (dos_header->e_magic != pe::IMAGE_DOS_SIGNATURE) {
        return framework::err(framework::status_assert_failed);
    }

    const auto* nt_headers = first_page.template data<pe::ImageNtHeaders64>(dos_header->e_lfanew);
    if (nt_headers->Signature != pe::IMAGE_NT_SIGNATURE) {
        return framework::err(framework::status_assert_failed);
    }

    const framework::span headers{image_base, nt_headers->OptionalHeader.SizeOfHeaders};
    auto buffer = framework::buffer::from(headers);
    return framework::ok(framework::move(buffer));
}

template<memory::memory_mapper_type mem_t_>
framework::result<framework::buffer> load_module_exception_table(memory::memory_mapper<mem_t_>& memory_mapper, const pe::headers& headers) {
    const auto base = reinterpret_cast<uint64_t>(headers.base());
    const auto* directory = headers.data_directory(pe::DataDirectoryType::image_directory_entry_exception);
    if (directory->VirtualAddress == 0 || (directory->VirtualAddress - base) >= headers.image_size()) {
        return framework::err(framework::status_not_found);
    }

    const auto section = verify(memory_mapper->map(base + directory->VirtualAddress, directory->Size));

    auto buffer = framework::buffer::create(section.data(), section.size());
    return framework::ok(framework::move(buffer));
}

template<memory::memory_mapper_type mem_t_>
framework::result<framework::buffer> load_module_name(memory::memory_mapper<mem_t_>& memory_mapper, const void* image_base) {

}

template<memory::memory_mapper_type mem_t_>
framework::result<loaded_module> load_module(memory::memory_mapper<mem_t_>& memory_mapper, const void* image_base) {
    auto headers_buffer = verify(load_module_headers(memory_mapper, image_base));
    auto exception_table_buffer = verify(load_module_exception_table(memory_mapper, image_base));

    return framework::ok(loaded_module{framework::move(headers_buffer), framework::move(exception_table_buffer), });
}

static const loaded_module* find_module(loaded_modules& modules, const void* rip) {
    if (const auto* module = modules.find_module(rip)) {
        return module;
    }

    const auto start_address = framework::round_up(reinterpret_cast<uint64_t>(rip), x86::paging::page_size);
    // only search backwards up to 2MB, we don't want to search forever
    const auto end_address = start_address - x86::paging::page_size_2m;
    for (auto address = start_address; address >= end_address; address -= x86::paging::page_size) {
        const auto* header = reinterpret_cast<const pe::ImageDosHeader*>(address);
        if (header->e_magic == pe::IMAGE_DOS_SIGNATURE) {
            // found pe signature, but need to make sure it is actually valid, which the following function
            // will check
            const auto* module = modules.add_if_image_base(reinterpret_cast<void*>(address));
            if (module != nullptr) {
                return module;
            }
        }
    }

    return nullptr;
}

function_entry::function_entry(const pe::section_list& sections, const pe::function_entry& entry)
    : entry(entry)
    , start(sections.rva_to_pointer<void>(entry.start()))
    , end(sections.rva_to_pointer<void>(entry.end()))
    , unwind_info(entry.find_unwind_info(sections))
{}

loaded_module::loaded_module(framework::buffer&& headers_data, framework::buffer&& functions_data, framework::buffer&& name_data)
    : m_headers_data(framework::move(headers_data))
    , m_functions_data(framework::move(functions_data))
    , m_name_data(framework::move(name_data))
    , m_headers(m_headers_data.data())
{}

const void* loaded_module::base() const {
    return m_headers.base();
}

const void* loaded_module::end() const {
    return m_headers.base() + m_headers.image_size();
}

size_t loaded_module::size() const {
    return m_headers.image_size();
}

const char* loaded_module::name() const {
    return m_name_data.data<char>();
}

bool loaded_module::contains(const void* ptr) const {
    return ptr >= base() && ptr < end();
}

framework::optional<function_entry> loaded_module::find_function(const void* ptr) const {
    if (!contains(ptr)) {
        // todo: error
        return framework::nullopt;
    }


    const auto* function_table = m_functions_data.data<pe::ImageRuntimeFunctionEntry>();

    /*binary search
    *    int low = 0, high = size - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2; // Prevents overflow
        if (arr[mid] == target) return mid;
        if (arr[mid] < target) low = mid + 1;
        else high = mid - 1;
    }
    return -1;
     */

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

const loaded_module* loaded_modules::add_if_image_base(const void* base) {
    if (pe::image::is_image_base(base)) {
        m_modules.push_back(loaded_module(base));
        return &m_modules.back();
    }

    return nullptr;
}

framework::result<stack_frame> unwind_next(const loaded_module* module, const stack_frame& current) {
    const auto function_opt = module->find_function(current.rip);
    if (!function_opt) {
        // trace_debug("did not find function for 0x%p in module 0x%p", current.rip, module->base());
        return framework::err(framework::status_not_found);
    }

    const auto unwind_info = function_opt.value().unwind_info;
    const auto unwind_fr = unwind_info.frame_register();

    const auto rsp = verify(calculate_original_rsp(unwind_info, current));
    const auto return_address = *static_cast<const uint64_t*>(rsp);

    stack_frame next_frame{
        reinterpret_cast<void*>(return_address),
        nullptr,
        static_cast<uint8_t*>(rsp) + 8};

    if (static_cast<pe::UnwindCodeOpInfo>(unwind_fr) == pe::uwinfo_rbp) {
        const auto last_rbp = *reinterpret_cast<uint64_t*>(reinterpret_cast<uint64_t>(rsp) - 8);
        next_frame.rbp = reinterpret_cast<void*>(last_rbp);
    }

    return framework::ok(next_frame);
}

framework::result<> print_stack_frame(loaded_modules& modules, stack_frame current) {
    char buffer[512];
    size_t offset = 0;
    size_t buffer_size = sizeof(buffer);

    ascii_format(buffer, offset, buffer_size, "Stack Unwind:\n");
    do {
        const auto* module = find_module(modules, current.rip);
        if (module == nullptr) {
            break;
        }

        ascii_format(buffer, offset, buffer_size, "\tFrame: [Module %a (0x%p -> 0x%p)] rip=0x%p, rbp=0x%p, rsp=0x%p\n",
                module->name(), module->base(), module->end(),
                current.rip, current.rbp, current.rsp);

        const auto result = unwind_next(module, current);
        if (!result) {
            break;
        }

        current = result.value();
    } while (true);

    buffer[offset] = '\0';
    trace_debug("%a", buffer);

    return {};
}

framework::result<> print_stack_frame(loaded_modules& modules, const uint64_t rip, const uint64_t rbp, const uint64_t rsp) {
    return print_stack_frame(modules, stack_frame{
        reinterpret_cast<void*>(rip), reinterpret_cast<void*>(rbp), reinterpret_cast<void*>(rsp)});
}

}
