
#include <x86/msr.h>
#include <environment.h>

#include "efi_base.h"

namespace framework::environment {

struct mp_procedure_context_t {
    vcpu_procedure_t* procedure;
    void* param;
};

static void mp_procedure(void* param) {
    // TODO MAKE ATOMIC STUFF
    // TODO HOW TO MARK FAILURE HERE?
    const auto context = static_cast<mp_procedure_context_t*>(param);
    const auto status = context->procedure(context->param);
    if (!status) {
        trace_error("Failed to run procedure on core");
        trace_status(status.error());
    }
}

result<void*> allocate_pages(const size_t pages, const memory_type_t type) {
    EFI_MEMORY_TYPE efi_memory_type;
    switch (type) {
        case memory_type_t::code:
            efi_memory_type = EfiRuntimeServicesCode;
            break;
        case memory_type_t::data:
            efi_memory_type = EfiRuntimeServicesData;
            break;
        default:
            return framework::err(framework::status_bad_arg);
    }

    EFI_PHYSICAL_ADDRESS address;
    verify_efi(gBS->AllocatePages(AllocateAnyPages, efi_memory_type, pages, &address));

    return framework::ok(to_virtual(address));
}

void free_pages(void* ptr, const size_t pages, const memory_type_t) {
    const auto address = to_physical(ptr);
    gBS->FreePages(address, pages);
}

physical_address_t to_physical(const void* address) {
    // 1-to-1 mapping because of UEFI
    return reinterpret_cast<physical_address_t>(address);
}

void* to_virtual(const physical_address_t address) {
    // 1-to-1 mapping because of UEFI
    return reinterpret_cast<void*>(address);
}

size_t get_current_vcpu_id() {
    const auto fs_base = x86::read<x86::msr::ia32_fs_base_t>();
    return fs_base.raw;
}

void set_current_vcpu_id(const size_t id) {
    x86::msr::ia32_fs_base_t fs_base{};
    fs_base.raw = id;
    x86::write<x86::msr::ia32_fs_base_t>(fs_base);
}

result<size_t> get_active_cpu_count() {
    EFI_MP_SERVICES_PROTOCOL* mp_services;
    verify_efi(gBS->LocateProtocol(&gEfiMpServiceProtocolGuid, nullptr, reinterpret_cast<void**>(&mp_services)));

    uintn_t cpu_count;
    uintn_t enabled_cpu_count;
    verify_efi(mp_services->GetNumberOfProcessors(mp_services, &cpu_count, &enabled_cpu_count));

    return framework::ok(enabled_cpu_count);
}

framework::result<> run_on_all_vcpu(vcpu_procedure_t procedure, void* param) {
    EFI_MP_SERVICES_PROTOCOL* mp_services;
    verify_efi(gBS->LocateProtocol(&gEfiMpServiceProtocolGuid, nullptr, reinterpret_cast<void**>(&mp_services)));

    mp_procedure_context_t context{};
    context.procedure = procedure;
    context.param = param;

    // run on bsp first
    mp_procedure(&context);

    const auto status = mp_services->StartupAllAPs(mp_services, mp_procedure, true, &context, 0, nullptr, nullptr);
    if (EFI_ERROR(status) && status != EFI_NOT_STARTED) {
        // EFI_NOT_STARTED = no other APs are started, likely only because they do not exist and
        //  this system only has one.
        verify_efi(status);
    }

    return {};
}

result<> sleep(const size_t microseconds) {
    verify_efi(gBS->Stall(microseconds));
    return {};
}

void do_abort() {
    // todo: implement
}

}
