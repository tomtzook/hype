
#include "cpu.h"

extern "C" void asm_cpu_store_registers(hype::cpu_registers_t& registers);
extern "C" void asm_cpu_load_registers(const hype::cpu_registers_t& registers);

namespace hype {

void read_registers(cpu_registers_t& registers) {
    asm_cpu_store_registers(registers);
}

void write_registers(const cpu_registers_t& registers) {
    asm_cpu_load_registers(registers);
}

}
