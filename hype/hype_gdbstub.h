#pragma once

#include <x86/interrupts.h>

#include "cpu.h"

namespace gdbstub {

void initialize();
void handle(x86::interrupts::interrupt_t vector, hype::cpu_registers_t& registers);

}
