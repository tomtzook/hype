#pragma once

#include "vcpu.h"

namespace hype {

class environment_t {
public:
    environment_t() noexcept
        : m_vcpu_service()
    {}
    ~environment_t() noexcept = default;

    vcpu_service_t& get_vcpu_service() noexcept;

    friend common::result initialize(environment_t& environment) noexcept;
private:
    vcpu_service_t m_vcpu_service;
};

}
