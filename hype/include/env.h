#pragma once

#include "vcpu.h"
#include "commonefi.h"


namespace hype {

class environment_t;

common::result initialize(environment_t& environment) noexcept;

class environment_t {
public:
    environment_t() noexcept
        : m_vcpu_service()
    {}
    ~environment_t() noexcept = default;

    vcpu_service_t& get_vcpu_service() noexcept;
    efi::efi_service_t& get_efi_service() noexcept;

    friend common::result hype::initialize(environment_t& environment) noexcept;
private:
    vcpu_service_t m_vcpu_service;
    efi::efi_service_t m_efi_service;
};

}
