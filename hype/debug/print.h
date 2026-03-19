#pragma once

namespace hype::debug {

void ascii_format(char* buffer, size_t& offset, size_t& buffer_size, const char* fmt, ...);
void memdump(const void* data, size_t length);

}
