
#include <x86/msr.h>

#include "common.h"

#include "vcpu.h"


static constexpr hype::vcpuid_t BSP_CPU_ID = 1;


static hype::vcpuid_t get_current_cpu_id_global() noexcept {
    return static_cast<hype::vcpuid_t>(x86::msr::read(x86::msr::ia32_fs_base_t::ID));
}

hype::vcpuid_t hype::vcpu_t::get_id() const noexcept {
    return get_current_cpu_id_global();
}

bool hype::vcpu_t::is_bootstrap() const noexcept {
    return BSP_CPU_ID == get_id();
}

hype::vcpu_service_t::vcpu_service_t() noexcept
    : m_vcpus(nullptr)
    , m_vcpus_count(0) {

}

hype::vcpu_service_t::~vcpu_service_t() noexcept {
    delete[] m_vcpus;
}

hype::vcpuid_t hype::vcpu_service_t::get_current_vcpu_id() noexcept {
    return get_current_cpu_id_global();
}

hype::vcpu_t& hype::vcpu_service_t::get_current_vcpu() noexcept {
    vcpuid_t vcpuid = get_current_vcpu_id();
    return m_vcpus[vcpuid];
}

size_t hype::vcpu_service_t::get_vcpu_count() noexcept {
    return m_vcpus_count;
}

common::result hype::vcpu_service_t::run_on_each_vcpu(vcpu_procedure_t procedure, void* param) noexcept {
#ifdef _MP
    // can use MpServices from EFI if before exit boot services.
    return hype::result::UNSUPPORTED_FEATURE;
#else
    // single processor
    return procedure(param);
#endif
}

common::result hype::initialize(vcpu_service_t& service) noexcept {
#ifdef _MP
    // can use MpServices from EFI if before exit boot services.
    return hype::result::UNSUPPORTED_FEATURE;
#else
    x86::msr::write(x86::msr::ia32_fs_base_t::ID, BSP_CPU_ID);
    return hype::result::SUCCESS;
#endif
}
