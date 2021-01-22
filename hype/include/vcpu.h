#pragma once

#include <x86/memory.h>
#include <x86/vmx/environment.h>
#include <x86/vmx/operations.h>


namespace hype {

using vcpuid_t = uintn_t;
using vcpu_procedure_t = common::result(void* param) noexcept;

struct vcpu_t {
    x86::vmx::vmxon_region_t vmxon_region PAGE_ALIGNED;

    bool is_in_vmx_operation;
};

class vcpu_service_t {
public:
    vcpu_service_t() noexcept;
    ~vcpu_service_t() noexcept;

    vcpuid_t get_current_vcpu_id() noexcept;
    vcpu_t& get_current_vcpu() noexcept;

    size_t get_vcpu_count() noexcept;

    common::result run_on_each_vcpu(vcpu_procedure_t procedure, void* param = nullptr) noexcept;

    friend common::result initialize(vcpu_service_t& service) noexcept;
private:
    vcpu_t* m_vcpus PAGE_ALIGNED;
    size_t m_vcpus_count;
};

}
