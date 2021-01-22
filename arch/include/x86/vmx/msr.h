#pragma once

#include <types.h>

#include "x86/msr.h"


namespace x86::msr {

struct ia32_vmx_basic_t {
    static const id_t ID = 0x480;
    union {
        struct {
            uint64_t vmcs_revision : 31;
            uint64_t must_be_zero : 1;
            uint64_t vm_struct_size : 13;
            uint64_t reserved0 : 3;
            uint64_t physaddr_width_type : 1;
            uint64_t dual_monitor_smi : 1;
            uint64_t vmcs_mem_type : 3;
            uint64_t ins_outs_vmexit_report : 1;
            uint64_t vm_ctrls_fixed : 1;
            uint64_t reserved1 : 8;
        } bits;
        uint64_t raw;
    };
} PACKED;

struct ia32_vmx_cr0_fixed0_t {
    static const id_t ID = 0x486;
    union {
        uint64_t raw;
    };
} PACKED;

struct ia32_vmx_cr0_fixed1_t {
    static const id_t ID = 0x487;
    union {
        uint64_t raw;
    };
} PACKED;

struct ia32_vmx_cr4_fixed0_t {
    static const id_t ID = 0x488;
    union {
        uint64_t raw;
    };
} PACKED;

struct ia32_vmx_cr4_fixed1_t {
    static const id_t ID = 0x489;
    union {
        uint64_t raw;
    };
} PACKED;


STATIC_ASSERT_SIZE(ia32_vmx_basic_t, 8);
STATIC_ASSERT_SIZE(ia32_vmx_cr0_fixed0_t, 8);
STATIC_ASSERT_SIZE(ia32_vmx_cr0_fixed1_t, 8);
STATIC_ASSERT_SIZE(ia32_vmx_cr4_fixed0_t, 8);
STATIC_ASSERT_SIZE(ia32_vmx_cr4_fixed1_t, 8);

}
