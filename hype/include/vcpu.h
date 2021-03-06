#pragma once

#include <x86/memory.h>
#include <x86/vmx/environment.h>
#include <x86/vmx/operations.h>
#include <x86/vmx/vmcs.h>


namespace hype {

using vcpuid_t = uintn_t;
using vcpu_procedure_t = common::result(void* param) noexcept;
class vcpu_service_t;

common::result initialize(vcpu_service_t& service) noexcept;

class vcpu_t {
public:
    vcpuid_t get_id() const noexcept;
    bool is_bootstrap() const noexcept;

    x86::vmx::vmxon_region_t vmxon_region PAGE_ALIGNED;
    x86::vmx::vmcs_t vmcs;

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

    friend common::result hype::initialize(vcpu_service_t& service) noexcept;
private:
    vcpu_t* m_vcpus PAGE_ALIGNED;
    size_t m_vcpus_count;
};

}
