#include <x86/cpuid.h>
#include <x86/segment.h>
#include <x86/regs.h>

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

    x86::cr0_t cr0;
    x86::load(cr0);
    TRACE_DEBUG("CR0=%x", cr0.raw);

    return EFI_SUCCESS;
}
