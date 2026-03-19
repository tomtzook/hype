
#include <efi/efi_base.h>
#include <base.h>

#include "print.h"

namespace hype::debug {

static bool isprint(const uint8_t c) {
    return c >= 0x20 && c <= 0x7E;
}

void ascii_format(char* buffer, size_t& offset, size_t& buffer_size, const char* fmt, ...) {
    VA_LIST args;
    VA_START(args, fmt);
    const auto written = AsciiVSPrint(buffer + offset, buffer_size, fmt, args);
    VA_END(args);

    offset += written;
    buffer_size -= written;
}

void memdump(const void* data, const size_t length) {
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

}
