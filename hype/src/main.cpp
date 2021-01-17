#include <x86/cpuid.h>
#include <x86/segment.h>
#include <x86/cr.h>
#include <x86/msr.h>

#include "commonefi.h"
#include "crt.h"
#include "environment.h"
#include "debug.h"


extern "C"
EFI_STATUS EFIAPI
efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table) {
    hype::result status = hype::result::SUCCESS;

    InitializeLib(image_handle, system_table);
    TRACE_DEBUG("Hello from the UEFI");

    x86::cpuid_regs_t cpuid_regs = {};
    x86::cpu_id(1, cpuid_regs);
    TRACE_CPUID_REG(1, 0, "EDX", cpuid_regs.edx);

    auto eax_01 = x86::cpu_id<x86::cpuid_eax01_t>(1);
    TRACE_CPUID_BIT(1, 0, "EDX", "PAE", eax_01.edx.bits.pae);

    x86::segment_table_t table{};
    x86::store(table);
    hype::debug::trace(table);

    x86::cr0_t cr0;
    x86::write(cr0);
    TRACE_REG_BIT("CR0", "PE", cr0.bits.paging_enable);

    auto efer = x86::msr::read<x86::msr::ia32_efer_t>(x86::msr::ia32_efer_t::ID);
    TRACE_REG_BIT("EFER", "LMA", efer.bits.lma);

    auto* cr0_n = new x86::cr0_t(0);
    VERIFY_ALLOCATION(cr0_n);
    TRACE_DEBUG("ALLOCATION s=%x", cr0_n);
    TRACE_DEBUG("plop");
    operator delete(cr0_n);

    TRACE_DEBUG("THIS IS THE END");
cleanup:
    TRACE_DEBUG("Finished with status: %d", status);
    hype::debug::deadloop();

    return EFI_SUCCESS;
}
