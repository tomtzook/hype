
#include <x86/msr.h>

#include "vcpu.h"


hype::vcpu_service_t::vcpu_service_t() noexcept
    : m_vcpus(nullptr)
    , m_vcpus_count(0) {

}

hype::vcpu_service_t::~vcpu_service_t() noexcept {
    delete[] m_vcpus;
}

hype::vcpuid_t hype::vcpu_service_t::get_current_vcpu_id() noexcept {
    return static_cast<vcpuid_t>(x86::msr::read(x86::msr::ia32_fs_base_t::ID));
}

hype::vcpu_t& hype::vcpu_service_t::get_current_vcpu() noexcept {
    vcpuid_t vcpuid = get_current_vcpu_id();
    return m_vcpus[vcpuid];
}

size_t hype::vcpu_service_t::get_vcpu_count() noexcept {
    return m_vcpus_count;
}

common::result hype::vcpu_service_t::run_on_each_vcpu(vcpu_procedure_t procedure, void* param) noexcept {
    // can use MpServices from EFI if before exit boot services.
}

common::result hype::initialize(vcpu_service_t& service) noexcept {

}

