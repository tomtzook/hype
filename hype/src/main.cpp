#include <x86/cpuid.h>
#include <x86/segment.h>

#include "commonefi.h"
#include "debug.h"

extern "C"
EFI_STATUS EFIAPI
efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table) {
    InitializeLib(image_handle, system_table);
    TRACE_DEBUG("Hello from the UEFI");

    x86::cpuid_regs_t cpuid_regs = {};
    x86::cpu_id(1, cpuid_regs);
    TRACE_DEBUG("CPUID[1].EAX=%x", cpuid_regs.eax);

    x86::segment_table_t table{};
    x86::store(table);
    hype::debug::trace(table);

    return EFI_SUCCESS;
}
