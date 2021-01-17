#include <x86/memory.h>

#include "commonefi.h"
#include "common.h"

#include <environment.h>


static const EFI_MEMORY_TYPE AllocationType = PoolAllocationType;

static size_t alignment_to_size(common::environment::alignment_t alignment) {
    switch (alignment) {
        case common::environment::alignment_t::PAGE_ALIGN: return x86::PAGE_SIZE;
        default: return 0;
    }
}


common::result common::environment::allocate(size_t size,
                                         void** out,
                                         alignment_t alignment) noexcept {
    // EfiRuntimeServicesData -> memory for runtime drivers.
    // This memory won't be deleted/overwritten when doing ExitBootServices. [UEFI-Specs 2.6 6.2 P152 "Note"]
    // Using it when subsystem=efi-app doesn't work, returning EFIERR(9).
    //
    // PoolAllocationType changes by subsystem. For subsystem=efi-app it's EfiLoaderData.
    // This actually works for allocation.
    // https://github.com/vathpela/gnu-efi/blob/master/lib/misc.c (::AllocatePool)
    EFI_STATUS status;
    void* memory;

    if (alignment_t::NO_ALIGN != alignment) {
        size_t alignment_size = alignment_to_size(alignment);
        status = uefi_call_wrapper((void*)BS->AllocatePool, 3, AllocationType, size + alignment_size - 1, &memory);
        if (EFI_ERROR(status)) {
            memory = nullptr;
        } else {
            memory = reinterpret_cast<void*>(((uintptr_t)memory + alignment_size - 1) & ~(uintptr_t)(alignment_size - 1));
        }
    } else {
        status = uefi_call_wrapper((void*)BS->AllocatePool, 3, AllocationType, size, &memory);
        if (EFI_ERROR(status)) {
            memory = nullptr;
        }
    }

    *out = memory;
    return hype::efi::efi_result(status);
}

common::result common::environment::free(void* memory) noexcept {
    if (nullptr == memory) {
        return hype::result::SUCCESS;
    }

    EFI_STATUS status = uefi_call_wrapper((void*)BS->FreePool, 1, memory);
    return hype::efi::efi_result(status);
}
