
#include <x86/regs.h>
#include <pe.h>
#include <environment.h>
#include <efi/efi_base.h>

#include "debug.h"

namespace hype {

struct frame {
    uint64_t rip;
    uint64_t rbp;
};

static framework::optional<pe::function_entry> find_function(const pe::image& image, const uint64_t offset) {
    const auto sections = image.sections();

    const auto table = image.load_exception_table();
    for (const auto& entry : table) {
        const auto start = sections.rva_to_offset(entry.start());
        const auto end = sections.rva_to_offset(entry.end());
        if (offset >= start && offset <= end) {
            return entry;
        }
    }

    return framework::nullopt;
}

framework::result<frame> unwind_next(const pe::image& image, const uint64_t rip, const uint64_t rbp) {
    const auto rip_offset = rip - reinterpret_cast<uint64_t>(image.headers().base());
    const auto func_opt = find_function(image, rip_offset);
    if (!func_opt) {
        trace_debug("function not found, offset=0x%x", rip_offset);
        return framework::err(framework::status_unsupported);
    }

    const auto unwind_info = func_opt.value().find_unwind_info(image.sections());
    const auto unwind_fr = unwind_info.frame_register();
    if (unwind_fr != 0 && static_cast<pe::UnwindCodeOpInfo>(unwind_fr) == pe::UWINFO_RBP) {
        const auto fp = rbp;
        const auto offset = unwind_info.frame_register_offset() * 16;
        const auto last_rbp = reinterpret_cast<uint64_t*>(fp - offset)[0];
        const auto return_address = reinterpret_cast<uint64_t*>(fp - offset + 8)[0];
        return framework::ok(frame{return_address, last_rbp});
    } else {
        // todo view unwind codes
        trace_debug("fr is not rbp: 0x%x", unwind_fr);
        return framework::err(framework::status_unsupported);
    }
}

framework::result<> unwind_our_image(uint64_t rip, uint64_t rbp) {
    const auto image_info = verify(environment::get_our_image_info());

    const pe::image image(image_info.base);
    trace_debug("Stack unwind: rip=0x%p, rbp=0x%p", rip, rbp);
    do {
        const auto result = unwind_next(image, rip, rbp);
        if (!result) {
            trace_result("stack unwind error", result);
            break;
        }

        rip = result.value().rip;
        rbp = result.value().rbp;
        trace_debug("\tFrame: rip=0x%p, rbp=0x%p", rip, rbp);
    } while (true);

    //const auto s = image.sections()[".text"];
    /*for (const auto& s : image.sections()) {
        trace_debug("%S 0x%x, 0x%x, 0x%x, 0x%x", s.name(), s.virtual_address(), s.virtual_size(), s.pointer_to_raw_data(), s.size_of_raw_data());
    }*/

    return {};
}

static bool isprint(const uint8_t c) {
    return c >= 0x20 && c <= 0x7E;
}

static void ascii_format(char* buffer, size_t& offset, size_t& buffer_size, const char* fmt, ...) {
    VA_LIST args;
    VA_START(args, fmt);
    const auto written = AsciiVSPrint(buffer + offset, buffer_size, fmt, args);
    VA_END(args);

    offset += written;
    buffer_size -= written;
}

void hexdump(const void* data, const size_t length) {
    char buffer[512];
    size_t offset = 0;
    size_t buffer_size = sizeof(buffer);

    const auto* ptr = static_cast<const uint8_t*>(data);
    ascii_format(buffer, offset, buffer_size, "Dump 0x%p -> 0x%p:\n", ptr, ptr + length);

    for (int i = 0; i < length; i += 16) {
        ascii_format(buffer, offset, buffer_size, "%04x ", ptr + i);

        for (int j = 0; j < 16; ++j) {
            const auto b = ptr[i + j];
            ascii_format(buffer, offset, buffer_size, "%02x ", b);
        }

        ascii_format(buffer, offset, buffer_size, " ");

        for (int j = 0; j < 16; ++j) {
            const auto b = ptr[i + j];
            ascii_format(buffer, offset, buffer_size, "%c", isprint(b) ? b : '.');
        }

        ascii_format(buffer, offset, buffer_size, "\n");
    }

    buffer[offset] = '\0';
    trace_debug("%a", buffer);
}

framework::result<> print_pe_information() {
    const auto image_info = verify(environment::get_our_image_info());
    const pe::image image(image_info.base);
    const auto sections = image.sections();

    /*for (const auto& section : sections) {
        trace_debug("section: %s, 0x%x 0x%x", section.name(), section.virtual_address(), section.virtual_size());
    }*/

    const auto d = image.headers().data_directory(pe::DataDirectoryType::IMAGE_DIRECTORY_ENTRY_EXCEPTION);
    const auto section = image.sections()[d->VirtualAddress];
    trace_debug("d: 0x%x, 0x%x, s: %a 0x%x", d->VirtualAddress, d->Size, section.name(), section.virtual_address());

    const auto directory = section.rva_to_pointer<pe::ImageRuntimeFunctionEntry>(d->VirtualAddress);
    trace_debug("0x%x 0x%x", directory->BeginAddress, directory->EndAddress);
    hexdump(directory, 16 * 3);


    return {};
}

void print_stack_info(const uint64_t rip, const uint64_t rbp) {
    const auto result = unwind_our_image(rip, rbp);
    if (!result) {
        trace_result("failed stack unwind", result);
    }
}

}
