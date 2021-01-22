
#include "common.h"

#include "env.h"


hype::vcpu_service_t& hype::environment_t::get_vcpu_service() noexcept {
    return m_vcpu_service;
}

common::result hype::initialize(environment_t& environment) noexcept {
    common::result status = hype::result::SUCCESS;

    CHECK(initialize(environment.m_vcpu_service));

cleanup:
    return status;
}
