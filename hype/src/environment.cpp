#include "commonefi.h"

#include "environment.h"
#include "debug.h"


hype::result hype::environment::allocate(size_t size,
                                         void** out,
                                         alignment_t alignment) {
    EFI_STATUS status;

    if (alignment_t::NO_ALIGN != alignment) {
        size_t alignment_size = static_cast<size_t>(alignment);
        status = gBS->AllocatePool(EfiRuntimeServicesData, size + alignment_size - 1, out);
        if (EFI_ERROR(status)) {
            *out = nullptr;
        } else {
            *out = reinterpret_cast<void*>(((uintptr_t)*out + alignment_size - 1) & ~(uintptr_t)(alignment_size - 1));
        }
    } else {
        status = gBS->AllocatePool(EfiRuntimeServicesData, size, out);
        if (EFI_ERROR(status)) {
            *out = nullptr;
        }
    }

    return hype::efi::efi_result(status);
}

hype::result hype::environment::free(void* memory) {
    if (nullptr == memory) {
        return hype::result::SUCCESS;
    }

    EFI_STATUS status = gBS->FreePool(memory);
    return hype::efi::efi_result(status);
}
