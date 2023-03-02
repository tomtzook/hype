
#include "commonefi.h"
#include "memory.h"


namespace hype::efi::memory {

static EFI_MEMORY_TYPE get_memory_type(hype::memory::type_t type) noexcept {
    switch (type) {
        case hype::memory::type_t::DATA: return EfiRuntimeServicesData;
        case hype::memory::type_t::CODE: return EfiRuntimeServicesCode;
        default: EfiRuntimeServicesData; // TODO: ASSERTION ERROR
    }
}

result::status allocate(void*& out, size_t size, hype::memory::type_t type) noexcept {

    EFI_STATUS status;
    void* memory;
    auto mem_type = get_memory_type(type);

    status = gBS->AllocatePool(mem_type, size, &memory);
    if (EFI_ERROR(status)) {
        memory = nullptr;
    }

    out = memory;
    return result::efi_status(status);
}

void free(void* ptr) noexcept {
    gBS->FreePool(ptr);
}

}
