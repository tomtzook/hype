#include <x86/cpuid.h>
#include <x86/segment.h>
#include <x86/cr.h>
#include <x86/msr.h>

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
    x86::write(cr0);
    TRACE_DEBUG("CR0=%x", cr0.raw);

    auto efer = x86::msr::read<x86::msr::ia32_efer_t>(x86::msr::ia32_efer_t::ID);
    TRACE_DEBUG("EFER.LMA=%d", efer.bits.lma);

    return EFI_SUCCESS;
}
