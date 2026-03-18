
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

static framework::result<size_t> find_setfp_code_index(const pe::unwind_info& unwind_info) {
    for (size_t i = 0; i < unwind_info.codes_count(); i++) {
        const auto code = unwind_info.code(i);

        const auto opcode = static_cast<pe::UnwindCodeOpCode>(code->u.OpCode);
        switch (opcode) {;
            case pe::UWOP_ALLOC_LARGE:
                if (code->u.OpInfo == 0) {
                    i += 1;
                } else {
                    i += 2;
                }
                break;
            case pe::UWOP_ALLOC_SMALL:
            case pe::UWOP_PUSH_NONVOL:
            case pe::UWOP_SAVE_NONVOL:
            case pe::UWOP_SAVE_NONVOL_FAR:
            case pe::UWOP_SAVE_XMM128:
            case pe::UWOP_SAVE_XMM128_FAR:
            case pe::UWOP_EPILOG:
            case pe::UWOP_SPARE_CODE:
            case pe::UWOP_PUSH_MACHFRAME:
                break;
            case pe::UWOP_SET_FPREG:
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
            case pe::UWOP_ALLOC_LARGE: {
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
            case pe::UWOP_ALLOC_SMALL: {
                const auto alloc_size = (code->u.OpInfo * 8) + 8;
                offset += alloc_size;
                break;
            }
            case pe::UWOP_PUSH_NONVOL:
            case pe::UWOP_SAVE_NONVOL:
            case pe::UWOP_SAVE_NONVOL_FAR:
                offset += 8;
                break;
            case pe::UWOP_SAVE_XMM128:
            case pe::UWOP_SAVE_XMM128_FAR:
                // todo: handle
                break;
            case pe::UWOP_EPILOG:
            case pe::UWOP_SPARE_CODE:
            case pe::UWOP_PUSH_MACHFRAME:
                // todo: handle
                break;
            case pe::UWOP_SET_FPREG:
                break;
        }
    }

    return framework::ok(offset);
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

framework::result<frame> unwind_next(const pe::image& image, const uint64_t rip, const uint64_t rbp) {
    const auto rip_offset = rip - reinterpret_cast<uint64_t>(image.headers().base());
    const auto func_opt = find_function(image, rip_offset);
    if (!func_opt) {
        return framework::err(framework::status_unsupported);
    }

    const auto unwind_info = func_opt.value().find_unwind_info(image.sections());
    const auto unwind_fr = unwind_info.frame_register();
    if (unwind_fr != 0 && static_cast<pe::UnwindCodeOpInfo>(unwind_fr) == pe::UWINFO_RBP) {
        // assuming that the function starts with push rbp
        const auto fp = rbp;
        const auto offset = unwind_info.frame_register_offset() * 16;
        const auto prolog_offset = verify(calc_prolog_rsp_offset(unwind_info));
        const auto last_rbp = reinterpret_cast<uint64_t*>(fp + prolog_offset - offset - 8)[0];
        const auto return_address = reinterpret_cast<uint64_t*>(fp + prolog_offset - offset)[0];
        return framework::ok(frame{return_address, last_rbp});
    }

    // todo implement
    return framework::err(framework::status_unsupported);
}

framework::result<> unwind_our_image(uint64_t rip, uint64_t rbp) {
    const auto image_info = verify(environment::get_our_image_info());
    const pe::image image(image_info.base, pe::memory_alignment::loaded);

    char buffer[512];
    size_t offset = 0;
    size_t buffer_size = sizeof(buffer);

    ascii_format(buffer, offset, buffer_size, "Stack Unwind:\n");
    ascii_format(buffer, offset, buffer_size, "\tFrame: rip=0x%p, rbp=0x%p\n", rip, rbp);
    do {
        const auto result = unwind_next(image, rip, rbp);
        if (!result) {
            break;
        }

        rip = result.value().rip;
        rbp = result.value().rbp;
        ascii_format(buffer, offset, buffer_size, "\tFrame: rip=0x%p, rbp=0x%p\n", rip, rbp);
    } while (true);

    buffer[offset] = '\0';
    trace_debug("%a", buffer);

    return {};
}

void print_stack_info(const uint64_t rip, const uint64_t rbp) {
    const auto result = unwind_our_image(rip, rbp);
    if (!result) {
        trace_result("failed stack unwind", result);
    }
}

}
