
#include <Uefi.h>
#include <Library/PrintLib.h>


extern "C"
EFI_STATUS EFIAPI
UefiMain(
        IN EFI_HANDLE ImageHandle,
        IN EFI_SYSTEM_TABLE* SystemTable
) {
    SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*) L"Hello");

    return EFI_SUCCESS;
}

