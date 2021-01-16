#pragma once

#include "types.h"


namespace x86::msr {

struct ia32_efer_t {
    static const uint32_t ID = 0xC0000080;
    union {
        struct {
            uint64_t sce : 1;
            uint64_t reserved0 : 7;
            uint64_t lme : 1;
            uint64_t reserved1 : 1;
            uint64_t lma : 1;
            uint64_t nxe : 1;
            uint64_t svme : 1;
            uint64_t lmsle : 1;
            uint64_t ffxsr : 1;
            uint64_t tce : 1;
            uint64_t reserved2 : 48;
        } bits;
        uint64_t raw;
    };
};


uint64_t read(uint32_t msr_id) noexcept;
void write(uint32_t msr_id, uint64_t value) noexcept;

template<typename msr_type>
msr_type read(uint32_t msr_id) noexcept;
template<typename msr_type>
void read(uint32_t msr_id, msr_type& msr) noexcept;
template<typename msr_type>
void write(uint32_t msr_id, const msr_type& msr) noexcept;

static_assert(sizeof(ia32_efer_t) == 8, "ia32_efer_t != 8");

}

template<typename msr_type>
msr_type x86::msr::read(uint32_t msr_id) noexcept {
    msr_type msr = {0};
    read(msr_id, msr);
    return msr;
}

template<typename msr_type>
void x86::msr::read(uint32_t msr_id, msr_type& msr) noexcept {
    uint64_t value = read(msr_id);
    auto& msr_r = reinterpret_cast<uint64_t&>(msr);
    msr_r = value;
}

template<typename msr_type>
void x86::msr::write(uint32_t msr_id, const msr_type& msr) noexcept {
    write(msr_id, reinterpret_cast<uint64_t>(msr));
}
