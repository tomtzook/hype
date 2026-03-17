
#include <pe.h>
#include <base.h>

#include "debug.h"

namespace hype {

static uint64_t read_frame_register(const pe::UnwindCodeOpInfo reg_type) {
    switch (reg_type) {
        case pe::UWINFO_RAX:
            break;
        case pe::UWINFO_RCX:
            break;
        case pe::UWINFO_RDX:
            break;
        case pe::UWINFO_RBX:
            break;
        case pe::UWINFO_RSP:
            break;
        case pe::UWINFO_RBP:
            break;
        case pe::UWINFO_RSI:
            break;
        case pe::UWINFO_RDI:
            break;
        case pe::UWINFO_R8:
            break;
        case pe::UWINFO_R9:
            break;
        case pe::UWINFO_R10:
            break;
        case pe::UWINFO_R11:
            break;
        case pe::UWINFO_R12:
            break;
        case pe::UWINFO_R13:
            break;
        case pe::UWINFO_R14:
            break;
        case pe::UWINFO_R15:
            break;
    }
}

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

void unwind(const pe::image& image, const uint64_t rip) {
    const auto rip_offset = rip - reinterpret_cast<uint64_t>(image.headers().base());
    const auto func_opt = find_function(image, rip_offset);
    if (!func_opt) {
        return;
    }

    const auto unwind_info = func_opt.value().find_unwind_info(image.sections());
    if (unwind_info.frame_register() != 0) {
        const auto fp = read_frame_register(static_cast<pe::UnwindCodeOpInfo>(unwind_info.frame_register()));
        const auto offset = unwind_info.frame_register_offset() * 16;
        const auto last_rbp = reinterpret_cast<uint64_t*>(fp - offset)[0];
        const auto return_address = reinterpret_cast<uint64_t*>(fp - offset + 8)[0];
    } else {
        // todo view unwind codes
    }
}

}
