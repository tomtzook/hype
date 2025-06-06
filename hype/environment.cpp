
#include <x86/msr.h>

#include "base.h"
#include "efi_base.h"

#include "environment.h"


namespace hype::environment {

struct mp_procedure_context_t {
    vcpu_procedure_t* procedure;
    void* param;
};

static void mp_procedure(void* param) {
    // TODO MAKE ATOMIC STUFF
    // TODO HOW TO MARK FAILURE HERE?
    const auto context = static_cast<mp_procedure_context_t*>(param);
    auto status = context->procedure(context->param);
    if (!status) {
        TRACE_ERROR("Failed to run procedure on core");
        TRACE_STATUS(status);
    }
}

status_t allocate_pages(void*& out, size_t pages, memory::memory_type_t memory_type) {
    EFI_MEMORY_TYPE efi_memory_type;
    switch (memory_type) {
        case memory::memory_type_t::code:
            efi_memory_type = EfiRuntimeServicesCode;
            break;
        case memory::memory_type_t::data:
            efi_memory_type = EfiRuntimeServicesData;
            break;
        default:
            CHECK(status::error_bad_argument);
    }

    EFI_PHYSICAL_ADDRESS address;
    CHECK(status::category_t::efi, gBS->AllocatePages(AllocateAnyPages, efi_memory_type, pages, &address));

    out = to_virtual(address);
    return {};
}

void free_pages(void* ptr, size_t pages) {
    auto address = to_physical(ptr);
    gBS->FreePages(address, pages);
}

physical_address_t to_physical(void* address) {
    // 1-to-1 mapping because of UEFI
    return reinterpret_cast<physical_address_t>(address);
}

void* to_virtual(physical_address_t address) {
    // 1-to-1 mapping because of UEFI
    return reinterpret_cast<void*>(address);
}

size_t get_current_vcpu_id() {
    auto fs_base = x86::read<x86::msr::ia32_fs_base_t>();
    return fs_base.raw;
}

void set_current_vcpu_id(const size_t id) {
    x86::msr::ia32_fs_base_t fs_base{};
    fs_base.raw = id;
    x86::write<x86::msr::ia32_fs_base_t>(fs_base);
}

status_t get_active_cpu_count(size_t& count) {
    EFI_MP_SERVICES_PROTOCOL* mp_services;
    CHECK(status::category_t::efi, gBS->LocateProtocol(&gEfiMpServiceProtocolGuid, nullptr, reinterpret_cast<void**>(&mp_services)));

    uintn_t cpu_count;
    uintn_t enabled_cpu_count;
    CHECK(status::category_t::efi, mp_services->GetNumberOfProcessors(mp_services, &cpu_count, &enabled_cpu_count));

    count = enabled_cpu_count;
    return {};
}

status_t run_on_all_vcpu(vcpu_procedure_t procedure, void* param) {
    EFI_MP_SERVICES_PROTOCOL* mp_services;
    CHECK(status::category_t::efi, gBS->LocateProtocol(&gEfiMpServiceProtocolGuid, nullptr, reinterpret_cast<void**>(&mp_services)));

    mp_procedure_context_t context{};
    context.procedure = procedure;
    context.param = param;

    // run on bsp first
    mp_procedure(&context);

    auto status = mp_services->StartupAllAPs(mp_services, mp_procedure, true, &context, 0, nullptr, nullptr);
    if (EFI_ERROR(status) && status != EFI_NOT_STARTED) {
        // EFI_NOT_STARTED = no other APs are started, likely only because they do not exist and
        //  this system only has one.
        CHECK(status::category_t::efi, status);
    }

    return {};
}

status_t sleep(size_t microseconds) {
    CHECK(status::category_t::efi, gBS->Stall(microseconds));
    return {};
}

}
