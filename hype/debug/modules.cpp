
#include <base.h>

#include "context.h"
#include "print.h"
#include "modules.h"

namespace hype::debug {

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
framework::result<memory::mapped_memory<mem_t_>> load_module_headers(memory::memory_mapper<mem_t_>& memory_mapper, const void* image_base) {
    auto first_page = verify(memory_mapper.map(reinterpret_cast<uint64_t>(image_base), x86::paging::page_size_4k));
    const auto* dos_header = first_page.template data<pe::ImageDosHeader>();
    if (dos_header->e_magic != pe::IMAGE_DOS_SIGNATURE) {
        return framework::err(framework::status_assert_failed);
    }

    const auto* nt_headers = first_page.template data<pe::ImageNtHeaders64>(dos_header->e_lfanew);
    if (nt_headers->Signature != pe::IMAGE_NT_SIGNATURE) {
        return framework::err(framework::status_assert_failed);
    }

    return framework::ok(framework::move(first_page));
}

template<memory::memory_mapper_type mem_t_>
framework::result<memory::mapped_memory<mem_t_>> load_module_exception_table(memory::memory_mapper<mem_t_>& memory_mapper, const void* image_base, const pe::headers& headers) {
    const auto guest_base = reinterpret_cast<uint64_t>(image_base);
    const auto* directory = headers.data_directory(pe::DataDirectoryType::image_directory_entry_exception);
    if (directory->VirtualAddress == 0) {
        return framework::err(framework::status_not_found);
    }

    auto section = verify(memory_mapper.map(guest_base + directory->VirtualAddress, directory->Size));
    return framework::ok(framework::move(section));
}

template<memory::memory_mapper_type mem_t_>
framework::result<framework::string> load_module_name_from_exports(memory::memory_mapper<mem_t_>& memory_mapper, const void* image_base, const pe::headers& headers) {
    const auto guest_base = reinterpret_cast<uint64_t>(image_base);
    const auto* directory = headers.data_directory(pe::DataDirectoryType::image_directory_entry_export);
    if (directory->VirtualAddress == 0) {
        return framework::err(framework::status_not_found);
    }

    uint32_t nameRva;
    {
        const auto export_directory = verify(memory_mapper.map(guest_base + directory->VirtualAddress, sizeof(pe::ImageExportDirectory)));
        nameRva = export_directory.template data<pe::ImageExportDirectory>()->Name;
    }
    if (nameRva == 0) {
        return framework::err(framework::status_not_found);
    }

    static constexpr auto max_str_len = 256;
    auto name = verify(memory_mapper.map(guest_base + nameRva, max_str_len));
    const auto len = framework::strlen_s(name.template data<char>(), max_str_len);

    auto str = verify(framework::string::from(name.template data<char>(), 0, len));
    return framework::ok(framework::move(str));
}

template<memory::memory_mapper_type mem_t_>
framework::result<framework::string> load_module_name_from_debug_table(memory::memory_mapper<mem_t_>& memory_mapper, const void* image_base, const pe::headers& headers) {
    const auto guest_base = reinterpret_cast<uint64_t>(image_base);
    const auto* directory = headers.data_directory(pe::DataDirectoryType::image_directory_entry_debug);
    if (directory->VirtualAddress == 0) {
        return framework::err(framework::status_not_found);
    }

    const auto debug_table = verify(memory_mapper.map(guest_base + directory->VirtualAddress, sizeof(pe::ImageDebugDirectory)));
    if (static_cast<pe::DebugType>(debug_table.template data<pe::ImageDebugDirectory>()->Type) == pe::DebugType::image_debug_type_codeview) {
        static constexpr auto max_str_len = 256;

        const auto debug_data_rva = debug_table.template data<pe::ImageDebugDirectory>()->AddressOfRawData;
        const auto debug_data = verify(memory_mapper.map(guest_base + debug_data_rva, sizeof(pe::CvInfoPdb70) + max_str_len));
        const auto pdb_file_name = debug_data.template data<pe::CvInfoPdb70>()->PdbFileName;

        const auto len = framework::strlen_s(pdb_file_name, max_str_len);
        auto str = verify(framework::string::from(pdb_file_name, 0, len));

        const auto last_slash = str.find_last('/');
        if (last_slash != framework::string::npos) {
            str = verify(str.substr(last_slash + 1));
        }

        return framework::ok(framework::move(str));
    }

    return framework::err(framework::status_not_found);
}

template<memory::memory_mapper_type mem_t_>
framework::result<framework::string> load_module_name(memory::memory_mapper<mem_t_>& memory_mapper, const void* image_base, const pe::headers& headers) {
    {
        auto result = load_module_name_from_exports(memory_mapper, image_base, headers);
        if (result) {
            return result;
        }
    }
    {
        auto result = load_module_name_from_debug_table(memory_mapper, image_base, headers);
        if (result) {
            return result;
        }
    }

    return framework::err(framework::status_not_found);
}

template<memory::memory_mapper_type mem_t_>
framework::result<loaded_module> load_module(memory::memory_mapper<mem_t_>& memory_mapper, const void* image_base) {
    auto headers = verify(load_module_headers(memory_mapper, image_base));
    auto exception_table = verify(load_module_exception_table(memory_mapper, image_base, pe::headers(headers.data())));

    auto module_name_result = load_module_name(memory_mapper, image_base, pe::headers(headers.data()));
    auto module_name_opt = module_name_result.is_success() ? framework::optional(framework::move(module_name_result.release_value())) : framework::nullopt;

    return framework::ok(loaded_module{memory_mapper, image_base, framework::move(headers), framework::move(exception_table), framework::move(module_name_opt)});
}

template<memory::memory_mapper_type mem_t_>
framework::result<const loaded_module&> find_module(memory::memory_mapper<mem_t_>& memory_mapper, loaded_modules& modules, const void* rip) {
    {
        auto result = modules.find_module(rip);
        if (result) {
            return result;
        }
    }

    const auto start_address = framework::round_up(reinterpret_cast<uint64_t>(rip), x86::paging::page_size);
    // only search backwards up to 2MB, we don't want to search forever
    const auto end_address = start_address - x86::paging::page_size_2m;
    for (auto address = start_address; address >= end_address; address -= x86::paging::page_size) {
        auto mapped_start = verify(memory_mapper.map(address, sizeof(pe::ImageDosHeader)));
        const auto* header = mapped_start.template data<pe::ImageDosHeader>();
        if (header->e_magic == pe::IMAGE_DOS_SIGNATURE) {
            // found pe signature, but need to make sure it is actually valid, which the following function
            // will check
            auto result = modules.add(reinterpret_cast<void*>(address));
            if (result) {
                return result;
            }
        }
    }

    return framework::err(framework::status_not_found);
}

function_entry::function_entry(const void* guest_image_base, const pe::ImageRuntimeFunctionEntry& function_entry, memory::mapped_memory<mem_t_>&& unwind_info_map)
    : guest_start(static_cast<const uint8_t*>(guest_image_base) + function_entry.BeginAddress)
    , guest_end(static_cast<const uint8_t*>(guest_image_base) + function_entry.EndAddress)
    , unwind_info_map(framework::move(unwind_info_map))
    , unwind_info(this->unwind_info_map.data<pe::UnwindInfo>())
{}

loaded_module::loaded_module(memory::memory_mapper<mem_t_> mapper, const void* guest_image_base, memory::mapped_memory<mem_t_>&& headers_data, memory::mapped_memory<mem_t_>&& functions_data, framework::optional<framework::string>&& name)
    : m_mapper(framework::move(mapper))
    , m_guest_image_base(guest_image_base)
    , m_headers_data(framework::move(headers_data))
    , m_functions_data(framework::move(functions_data))
    , m_name(framework::move(name))
    , m_headers(m_headers_data.data())
{}

const void* loaded_module::guest_base() const {
    return m_guest_image_base;
}

const void* loaded_module::guest_end() const {
    return static_cast<const uint8_t*>(m_guest_image_base) + m_headers.image_size();
}

size_t loaded_module::size() const {
    return m_headers.image_size();
}

const char* loaded_module::name() const {
    return m_name ? m_name->c_str() : "n/a";
}

bool loaded_module::contains(const void* guest_ptr) const {
    return guest_ptr >= guest_base() && guest_ptr < guest_end();
}

framework::result<function_entry> loaded_module::find_function(const void* guest_ptr) const {
    if (!contains(guest_ptr)) {
        return framework::err(framework::status_not_found);
    }

    const auto guest_image_base = static_cast<const uint8_t*>(guest_base());
    const auto offset = static_cast<const uint8_t*>(guest_ptr) - guest_image_base;
    const auto* function_table = m_functions_data.data<pe::ImageRuntimeFunctionEntry>();
    const auto size = m_functions_data.size() / sizeof(pe::ImageRuntimeFunctionEntry);

    for (int i = 0; i < size; ++i) {
        const auto& func = function_table[i];
        if (!func.BeginAddress) {
            break;
        }

        if (func.BeginAddress <= offset && func.EndAddress >= offset) {
            auto unwind_info = verify(m_mapper.map(reinterpret_cast<uint64_t>(guest_image_base + func.DUMMYUNIONNAME.UnwindInfoAddress), x86::paging::page_size));
            function_entry entry{guest_image_base, func, framework::move(unwind_info)};
            return framework::ok(framework::move(entry));
        }
    }

    return framework::err(framework::status_not_found);
}

loaded_modules::loaded_modules(memory::memory_mapper<mem_t_> mapper)
    : m_mapper(framework::move(mapper))
{}

framework::result<const loaded_module&> loaded_modules::find_module(const void* ptr) const {
    for (const auto& module : m_modules) {
        if (module.contains(ptr)) {
            return framework::ok<const loaded_module&>(module);
        }
    }

    return framework::err(framework::status_not_found);
}

framework::result<const loaded_module&> loaded_modules::add(const void* base) {
    auto module = verify(load_module(m_mapper, base));
    verify(m_modules.push_back(framework::move(module)));

    const auto& module_ref = m_modules.back();
    return framework::ok<const loaded_module&>{module_ref};
}

framework::result<stack_frame> unwind_next(memory::memory_mapper<memory::guest_memory_mapper>& mapper, const loaded_module& module, const stack_frame& current) {
    const auto function_opt = module.find_function(current.rip);
    if (!function_opt) {
        // trace_debug("did not find function for 0x%p in module 0x%p", current.rip, module->base());
        return framework::err(framework::status_not_found);
    }

    const auto unwind_info = function_opt.value().unwind_info;
    const auto unwind_fr = unwind_info.frame_register();

    const auto rsp = verify(calculate_original_rsp(unwind_info, current));

    auto return_address_mapping = verify(mapper.map(reinterpret_cast<uint64_t>(rsp), sizeof(uint64_t)));
    const auto return_address = *return_address_mapping.data<uint64_t>();

    stack_frame next_frame{
        reinterpret_cast<void*>(return_address),
        nullptr,
        static_cast<uint8_t*>(rsp) + 8};

    if (static_cast<pe::UnwindCodeOpInfo>(unwind_fr) == pe::uwinfo_rbp) {
        auto last_rbp_mapping = verify(mapper.map(reinterpret_cast<uint64_t>(rsp) - 8, sizeof(uint64_t)));
        const auto last_rbp = *last_rbp_mapping.data<uint64_t>();
        next_frame.rbp = reinterpret_cast<void*>(last_rbp);
    }

    return framework::ok(next_frame);
}

framework::result<> print_stack_frame(memory::memory_mapper<memory::guest_memory_mapper>& mapper, loaded_modules& modules, stack_frame current) {
    char buffer[512];
    size_t offset = 0;
    size_t buffer_size = sizeof(buffer);

    ascii_format(buffer, offset, buffer_size, "Stack Unwind:\n");
    do {
        const auto module_result = find_module(modules, current.rip);
        if (!module_result) {
            break;
        }

        const auto& module = module_result.value();
        ascii_format(buffer, offset, buffer_size, "\tFrame: [Module %a (0x%p -> 0x%p)] rip=0x%p, rbp=0x%p, rsp=0x%p\n",
                module.name(), module.guest_base(), module.guest_end(),
                current.rip, current.rbp, current.rsp);

        const auto result = unwind_next(mapper, module, current);
        if (!result) {
            break;
        }

        current = result.value();
    } while (true);

    buffer[offset] = '\0';
    trace_debug("%a", buffer);

    return {};
}

framework::result<> print_stack_frame(memory::memory_mapper<memory::guest_memory_mapper>& mapper, loaded_modules& modules, const uint64_t rip, const uint64_t rbp, const uint64_t rsp) {
    return print_stack_frame(mapper, modules, stack_frame{
        reinterpret_cast<void*>(rip), reinterpret_cast<void*>(rbp), reinterpret_cast<void*>(rsp)});
}

}
