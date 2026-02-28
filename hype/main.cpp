#include "efi/efi_base.h"
#include "x86/cpuid.h"

#include <base.h>
#include "cpu.h"
#include "environment.h"
#include "hype.h"
#include "hype_gdbstub.h"
#include "x86/io.h"


static void disable_qemu_timers() {
    // Offset for the LVT Timer Register is 0x320
    volatile uint32_t* lvt_timer = reinterpret_cast<uint32_t*>(0xFEE00000 + 0x320);
    // Set bit 16 to '1' to mask the interrupt
    *lvt_timer |= (1 << 16);
    // Port 0x21 is the Offset for the Master PIC Mask Register
    // Setting Bit 0 masks IRQ0 (the timer)
    x86::outb(0x21, x86::inb(0x21) | 0x01);
}

static framework::result<> init() {
    verify(environment::initialize());
    return {};
}

static framework::result<> start() {
    verify(hype::initialize());

    const auto start_result = hype::start();
    if (!start_result) {
        hype::free();

        return start_result;
    }

    return {};
}

extern "C"
EFI_STATUS EFIAPI
UefiMain(
        IN EFI_HANDLE ImageHandle,
        IN EFI_SYSTEM_TABLE* SystemTable
) {
    disable_qemu_timers();
    gdbstub::initialize();

    EFI_LOADED_IMAGE_PROTOCOL* loaded_image{};
    const auto _efiStatus = SystemTable->BootServices->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, reinterpret_cast<void**>(&loaded_image));
    if (_efiStatus != EFI_SUCCESS) {
        trace_error("Failed locating loaded image: 0x%lx", _efiStatus);
        return _efiStatus;
    }

    trace_debug("Main Start: base=0x%llx, end=0x%llx, size=0x%llx",
        loaded_image->ImageBase, reinterpret_cast<uint64_t>(loaded_image->ImageBase) + loaded_image->ImageSize, loaded_image->ImageSize);

    {
        const auto result = init();
        if (!result) {
            trace_status("initialization failed", result.error());
            return EFI_LOAD_ERROR;
        }
    }

    {
        const auto result = start();
        if (result) {
            auto res = x86::cpuid(1, 0);
            trace_debug("RES CPUID[rax=1, rcx=0]: eax=0x%lx, ebx=0x%lx, ecx=0x%lx, edx=0x%lx",
                res.eax, res.ebx, res.ecx, res.edx);

            trace_debug("Hypervisor Launched");
            __asm__ volatile ("cli; hlt");
        } else {
            trace_status("start failed", result.error());
            catastrophic_error("failed to start hypervisor")
        }
    }

    return EFI_SUCCESS;
}

