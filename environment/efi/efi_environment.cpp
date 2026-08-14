
#include <x86/msr.h>
#include <lock.h>

#include "efi_base.h"
#include "environment.h"


namespace framework {

void terminate() {
    // todo: implement
    __asm__ volatile ("cli; hlt");
}

}

namespace environment {

struct {
    EFI_HANDLE image_handle;
    void* image_base;
    size_t image_size;
} g_environment_data{};

struct mp_procedure_context_t {
    vcpu_procedure_t* procedure;
    void* param;
};

EFI_EXIT_BOOT_SERVICES g_exit_boot_services = nullptr;

static EFI_STATUS exit_boot_services(const EFI_HANDLE ImageHandle, const UINTN MapKey) {
    trace_debug("EXIT BOOT SERVICES");
    return g_exit_boot_services(ImageHandle, MapKey);
}

static void mp_procedure(void* param) {
    // TODO MAKE ATOMIC STUFF
    // TODO TO MARK FAILURE HERE?
    const auto context = static_cast<mp_procedure_context_t*>(param);
    const auto status = context->procedure(context->param);
    if (!status) {
        trace_status("Failed to run procedure on core", status.error());
    }

    trace_debug("Cpu procedure done");
}

static framework::result<> init_heap(const size_t pages, const framework::memory_type type) {
    const auto mem = verify(allocate_pages(pages, type));
    verify_status(efi::init_heap(type, mem, pages * x86::paging::page_size));

    return {};
}

framework::result<> initialize_efi(EFI_HANDLE image_handle) {
    g_environment_data.image_handle = image_handle;
    verify(initialize());
    return {};
}

framework::result<> initialize() {
    verify(init_heap(500, framework::memory_type::code));
    verify(init_heap(5000 + 500, framework::memory_type::data));

    EFI_LOADED_IMAGE_PROTOCOL* loaded_image{};
    const auto _efiStatus = gBS->HandleProtocol(
        g_environment_data.image_handle,
        &gEfiLoadedImageProtocolGuid,
        reinterpret_cast<void**>(&loaded_image));
    if (_efiStatus != EFI_SUCCESS) {
        verify_efi(_efiStatus);
    }
    g_environment_data.image_base = loaded_image->ImageBase;
    g_environment_data.image_size = loaded_image->ImageSize;

    // todo:
    //g_exit_boot_services = gBS->ExitBootServices;
    //gBS->ExitBootServices = exit_boot_services;

    return {};
}

framework::result<image_info> get_our_image_info() {
    return framework::ok(image_info{g_environment_data.image_base, g_environment_data.image_size});
}

framework::result<void*> allocate_pages(const size_t pages, const framework::memory_type type) {
    EFI_MEMORY_TYPE efi_memory_type;
    switch (type) {
        case framework::memory_type::code:
            efi_memory_type = EfiRuntimeServicesCode;
            break;
        case framework::memory_type::data:
            efi_memory_type = EfiRuntimeServicesData;
            break;
        default:
            return framework::err(framework::status_bad_arg);
    }

    EFI_PHYSICAL_ADDRESS address;
    verify_efi(gBS->AllocatePages(AllocateAnyPages, efi_memory_type, pages, &address));

    return framework::ok(to_virtual(address));
}

void free_pages(void* ptr, const size_t pages, const framework::memory_type) {
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

framework::result<size_t> get_active_cpu_count() {
    EFI_MP_SERVICES_PROTOCOL* mp_services;
    verify_efi(gBS->LocateProtocol(&gEfiMpServiceProtocolGuid, nullptr, reinterpret_cast<void**>(&mp_services)));

    uintn_t cpu_count;
    uintn_t enabled_cpu_count;
    verify_efi(mp_services->GetNumberOfProcessors(mp_services, &cpu_count, &enabled_cpu_count));

    return framework::ok(static_cast<size_t>(enabled_cpu_count));
}

framework::result<> run_on_all_vcpu(vcpu_procedure_t procedure, void* param) {
    EFI_MP_SERVICES_PROTOCOL* mp_services;
    verify_efi(gBS->LocateProtocol(&gEfiMpServiceProtocolGuid, nullptr, reinterpret_cast<void**>(&mp_services)));

    mp_procedure_context_t context{};
    context.procedure = procedure;
    context.param = param;

    // run on bsp
    mp_procedure(&context);

    const auto status = mp_services->StartupAllAPs(mp_services, mp_procedure, true, nullptr, 0, &context, nullptr);
    if (EFI_ERROR(status) && status != EFI_NOT_STARTED) {
        // EFI_NOT_STARTED = no other APs are started, likely only because they do not exist and
        //  this system only has one.
        verify_efi(status);
    }

    return {};
}

framework::result<> sleep(const size_t microseconds) {
    verify_efi(gBS->Stall(microseconds));
    return {};
}

framework::result<> serial_initialize() {
    verify_efi(SerialPortInitialize());
    return {};
}

constexpr auto COM2_BASE = 0x2F8;

framework::result<char> serial_read() {
    char ch;
    const auto read = SerialPortRead(reinterpret_cast<UINT8*>(&ch), 1);
    if (read == 0) {
        verify_efi(EFI_NOT_FOUND);
    }

    return framework::ok(ch);
}

framework::result<> serial_write(const char ch) {
    const auto written = SerialPortWrite(reinterpret_cast<UINT8*>(const_cast<char*>(&ch)), 1);
    assert(written == 1, "write failed for unknown reason");
    return {};
}

framework::result<bool> serial2_available() {
    return framework::ok((IoRead8(COM2_BASE + 5) & 0x01) != 0);
}

framework::result<char> serial2_read() {
    // Wait for Data Ready (DR) bit to be set
    while ((IoRead8(COM2_BASE + 5) & 0x01) == 0) {}
    const auto ch = static_cast<char>(IoRead8(COM2_BASE));

    return framework::ok(ch);
}

framework::result<> serial2_write(const char ch) {
    // Wait for Transmitter Holding Register Empty (THRE) bit to be set
    while ((IoRead8(COM2_BASE + 5) & 0x20) == 0) {}
    IoWrite8 (COM2_BASE, ch);

    return {};
}

bool g_trace_enabled_for_core[32]{true};

scoped_trace_disabler::scoped_trace_disabler() {
    set_trace_disabled_for_core();
}
scoped_trace_disabler::~scoped_trace_disabler() {
    set_trace_enabled_for_core();
}

void set_trace_enabled_for_core() {
    g_trace_enabled_for_core[get_current_vcpu_id()] = true;
}

void set_trace_disabled_for_core() {
    g_trace_enabled_for_core[get_current_vcpu_id()] = false;
}

}
