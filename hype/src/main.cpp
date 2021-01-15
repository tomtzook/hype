#include <efi.h>
#include <efilib.h>
#include <cstddef>

#include "types.h"
#include "x86/cpuid.h"
#include "x86/regs.h"
#include "x86/segment.h"


extern "C"
EFI_STATUS EFIAPI
efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table) {
    InitializeLib(image_handle, system_table);

    x86::cpuid_regs_t regs = {0};
    x86::cpu_id(1, regs);

    x86::cr0_t cr0(0);
    cr0.bits.cache_disable = true;

    x86::segment_table64_t gdt;
    gdt.store();

    Print((CHAR16*)L"ADDR=%p ... LIMIT=%d\n",
          gdt.base_address(), gdt.limit());
    for (size_t i = 0; i < gdt.limit(); i++) {
        x86::segment_descriptor_t& descriptor = gdt[i];
        Print((CHAR16*)L"(%lx) address=%p, limit=%d\n",
              i, descriptor.base_address(), descriptor.limit());
    }

    return EFI_SUCCESS;
}
