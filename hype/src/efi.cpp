
#include "common.h"

#include "commonefi.h"


static const EFI_MEMORY_TYPE ALLOCATION_TYPE = PoolAllocationType;

static bool g_is_after_exit_boot_services = false;
static EFI_EXIT_BOOT_SERVICES g_original_exit_boot_services = nullptr;


common::result efi::result::efi_result(EFI_STATUS efi_status) noexcept {
    return common::result(efi_status, efi::result::CATEGORY);
}

template<>
common::result::result_t(EFI_STATUS code) noexcept
    : m_code(code)
    , m_category(efi::result::CATEGORY)
{}

common::result efi::allocate(size_t size, void** out) noexcept {
    // EfiRuntimeServicesData -> memory for runtime drivers.
    // This memory won't be deleted/overwritten when doing ExitBootServices. [UEFI-Specs 2.6 6.2 P152 "Note"]
    // Using it when subsystem=efi-app doesn't work, returning EFIERR(9).
    //
    // PoolAllocationType changes by subsystem. For subsystem=efi-app it's EfiLoaderData.
    // This actually works for allocation.
    // https://github.com/vathpela/gnu-efi/blob/master/lib/misc.c (::AllocatePool)
    EFI_STATUS status;
    void* memory;

    status = uefi_call_wrapper((void*)BS->AllocatePool, 3, ALLOCATION_TYPE, size, &memory);
    if (EFI_ERROR(status)) {
        memory = nullptr;
    }

    *out = memory;
    return efi::result::efi_result(status);
}

common::result efi::free(void* memory) noexcept {
    EFI_STATUS status = uefi_call_wrapper((void*)BS->FreePool, 1, memory);
    return efi::result::efi_result(status);
}

static EFI_STATUS exit_boot_services_hook(EFI_HANDLE image_handle, UINTN map_key) {
    g_is_after_exit_boot_services = true;
    return g_original_exit_boot_services(image_handle, map_key);
}

bool efi::efi_service_t::is_after_exit_boot_services() noexcept {
    return g_is_after_exit_boot_services;
}

common::result efi::initialize(efi_service_t& service) noexcept {
    g_original_exit_boot_services = gBS->ExitBootServices;
    gBS->ExitBootServices = exit_boot_services_hook;

    return common::result::SUCCESS;
}
