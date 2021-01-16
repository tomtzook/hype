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
    TRACE_DEBUG("CPUID[1].EDX=%x %d", cpuid_regs.edx,
                ((cpuid_regs.edx >> 6) & 0x1));

    x86::cpuid_eax01_t eax_01 = {};
    x86::cpu_id(1, eax_01);
    TRACE_DEBUG("CPUID[1].EDX.PAE=%x", eax_01.edx.bits.pae);

    x86::segment_table_t table{};
    x86::store(table);
    hype::debug::trace(table);

    x86::cr0_t cr0;
    x86::load(cr0);
    TRACE_DEBUG("CR0=%x", cr0.raw);


    return EFI_SUCCESS;
}
