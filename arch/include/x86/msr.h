#pragma once

#include <types.h>


namespace x86::msr {

using id_t = uint32_t;

struct ia32_efer_t {
    static constexpr id_t ID = 0xC0000080;
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
} PACKED;

struct ia32_feature_ctrl_t {
    static constexpr id_t ID = 0x3a;
    union {
        struct {
            uint64_t lock_bit : 1;
            uint64_t vmx_smx : 1;
            uint64_t vmx_no_smx : 1;
            uint64_t reserved10 : 61;
        } bits;
        uint64_t raw;
    };
} PACKED;

struct ia32_fs_base_t {
    static constexpr id_t ID = 0xC0000100;
    union {
        uint64_t raw;
    };
} PACKED;

uint64_t read(id_t msr_id) noexcept;
void write(id_t msr_id, uint64_t value) noexcept;

template<typename msr_type>
msr_type read(id_t msr_id) noexcept;
template<typename msr_type>
void read(id_t msr_id, msr_type& msr) noexcept;
template<typename msr_type>
void write(id_t msr_id, const msr_type& msr) noexcept;


STATIC_ASSERT_SIZE(ia32_efer_t, 8);
STATIC_ASSERT_SIZE(ia32_feature_ctrl_t, 8);
STATIC_ASSERT_SIZE(ia32_fs_base_t, 8);

}

template<typename msr_type>
msr_type x86::msr::read(id_t msr_id) noexcept {
    msr_type msr = {0};
    read(msr_id, msr);
    return msr;
}

template<typename msr_type>
void x86::msr::read(id_t msr_id, msr_type& msr) noexcept {
    uint64_t value = read(msr_id);
    auto& msr_r = reinterpret_cast<uint64_t&>(msr);
    msr_r = value;
}

template<typename msr_type>
void x86::msr::write(id_t msr_id, const msr_type& msr) noexcept {
    auto& msr_r = reinterpret_cast<const uint64_t&>(msr);
    write(msr_id, msr_r);
}
