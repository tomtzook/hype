#pragma once

#include "types.h"

namespace x86 {

struct cr0_t {
    union {
        struct {
            uint64_t pe : 1;
            uint64_t mp : 1;
            uint64_t em : 1;
            uint64_t ts : 1;
            uint64_t et : 1;
            uint64_t ne : 1;
            uint64_t reserved0 : 10;
            uint64_t wp : 1;
            uint64_t am : 1;
            uint64_t reserved1 : 10;
            uint64_t nw : 1;
            uint64_t cd : 1;
            uint64_t pg : 1;
        } bits;

        uint64_t raw;
    };

    cr0_t() noexcept;
    explicit cr0_t(uint64_t raw) noexcept;

    void clear() noexcept;
    void load() noexcept;
    void store() noexcept;
};

struct cr4_t {
    union {
        struct {
            uint64_t vme : 1;
            uint64_t pvi : 1;
            uint64_t tsd : 1;
            uint64_t de : 1;
            uint64_t pse : 1;
            uint64_t pae : 1;
            uint64_t mce : 1;
            uint64_t pge : 1;
            uint64_t pce : 1;
            uint64_t osfxsr : 1;
            uint64_t osxmmexcpt : 1;
            uint64_t umip : 1;
            uint64_t la57 : 1;
            uint64_t vmxe : 1;
            uint64_t smxe : 1;
            uint64_t fsgsbase : 1;
            uint64_t pcide : 1;
            uint64_t osxsave : 1;
            uint64_t smep : 1;
            uint64_t smap : 1;
            uint64_t pke : 1;
            uint64_t reserved0 : 10;
        } bits;

        uint64_t raw;
    };

    cr4_t() noexcept;
    explicit cr4_t(uint64_t raw) noexcept;

    void clear() noexcept;
    void load() noexcept;
    void store() noexcept;
};

}
