#pragma once

#include <x86/paging/ia32e.h>
#include <x86/vmx/ept.h>

namespace hype::debug {

void ascii_format(char* buffer, size_t& offset, size_t& buffer_size, const char* fmt, ...);
void memdump(const void* data, size_t length);
void instruction_dump(const void* data, size_t count);

void print_page_mapping_simple(const x86::cr3_t& cr3, x86::paging::ia32e::linear_address_t address);
void print_ept_mapping_simple(const x86::vmx::ept_pointer_t& eptp, x86::vmx::guest_physical_address_t address);

}
