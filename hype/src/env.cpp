
#include "common.h"

#include "env.h"


hype::vcpu_service_t& hype::environment_t::get_vcpu_service() noexcept {
    return m_vcpu_service;
}

efi::efi_service_t& hype::environment_t::get_efi_service() noexcept {
    return m_efi_service;
}

common::result hype::initialize(environment_t& environment) noexcept {
    common::result status;

    CHECK(initialize(environment.m_vcpu_service));
    CHECK(initialize(environment.m_efi_service));

cleanup:
    return status;
}
