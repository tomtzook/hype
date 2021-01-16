extern "C" {

#include <efi.h>
#include <efilib.h>

}

extern "C"
EFI_STATUS EFIAPI
efi_main (EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table)
{
    InitializeLib(image_handle, system_table);
    Print((CHAR16*)L"Hello from the UEFI\n");
    return EFI_SUCCESS;
}
