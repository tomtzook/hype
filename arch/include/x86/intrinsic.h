#pragma once

#include "types.h"


extern "C" uint64_t _read_cr0() noexcept;
extern "C" void _write_cr0(uint64_t value) noexcept;

extern "C" uint64_t _read_cr3() noexcept;
extern "C" void _write_cr3(uint64_t value) noexcept;

extern "C" uint64_t _read_cr4() noexcept;
extern "C" void _write_cr4(uint64_t value) noexcept;

extern "C" void _sgdt(void* value) noexcept;
extern "C" void _lgdt(void* value) noexcept;

extern "C" uint64_t _read_msr(uint32_t id) noexcept;
extern "C" void _write_msr(uint32_t id, uint64_t value) noexcept;

extern "C" void _cpuid(uint32_t leaf, uint32_t subleaf, uint32_t* regs) noexcept;
