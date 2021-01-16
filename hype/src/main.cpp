#include "commonefi.h"

#include <x86/cpuid.h>

extern "C"
EFI_STATUS EFIAPI
efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table)
{
    InitializeLib(image_handle, system_table);
    Print((CHAR16*)L"Hello from the UEFI\n");

    x86::cpuid_regs_t cpuid_regs = {};
    x86::cpu_id(1, cpuid_regs);

    Print((CHAR16*)L"CPUID[1].EAX=%x\n", cpuid_regs.eax);

    return EFI_SUCCESS;
}
