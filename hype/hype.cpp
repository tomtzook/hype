
#include <x86/paging/paging.h>

#include <environment/allocation.h>

#include "hype.h"


namespace hype {

context_t* g_context = nullptr;

static result::status check_environment_support() noexcept {
    // TODO: CHECK VMX SUPPORT
    // TODO: add check for secondary and primary controls
    // TODO: add check for unrestricted guest support

    CHECK_ASSERT(x86::paging::mode_t::ia32e == x86::paging::current_mode() &&
                 environment::are_huge_tables_supported(),
                 "required paging not supported");

    return {};
}


static result::status start_on_vcpu(void* param) noexcept {
    //common::result status;

    //hype::context_t* context = reinterpret_cast<hype::context_t*>(param);
    //hype::vcpu_t& cpu = context->environment.get_vcpu_service().get_current_vcpu();

    TRACE_DEBUG("Starting on core");

    //CHECK(x86::vmx::on(cpu.vmxon_region));
    //cpu.is_in_vmx_operation = true;

    //cpu.vmcs.clear();
    //CHECK(x86::vmx::write(cpu.vmcs));
    //CHECK(setup_vmcs(cpu, cpu.vmcs));
    //CHECK(x86::vmx::launch());
/*
cleanup:
    if (status && cpu.is_in_vmx_operation) {
        CHECK_SILENT(x86::vmx::off());
        cpu.is_in_vmx_operation = false;
    }

    return status;*/
}

result::status initialize() noexcept {
    CHECK_ASSERT(g_context == nullptr, "context not null");

    TRACE_DEBUG("Checking environment support");
    CHECK_AND_RETURN(check_environment_support());

    TRACE_DEBUG("Initializing context");
    result::status status = {};

    g_context = new (hype::memory::alignment_t::PAGE_ALIGN) context_t;
    CHECK_ALLOCATION_AND_JUMP(status, g_context, "context initialization failed");

    CHECK_AND_JUMP(status, environment::page_table::setup_identity_paging(g_context->page_table));
    // todo: initialize ept

cleanup:
    if (!status && g_context != nullptr) {
        ::operator delete(g_context);
        g_context = nullptr;
    }

    return status;
}

result::status start() noexcept {
    return {};
}

void free() noexcept {

}

}
