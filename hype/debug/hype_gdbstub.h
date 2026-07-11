#pragma once

#include <x86/interrupts.h>

#include "../cpu.h"

namespace gdbstub {

void initialize();
void handle_interrupt(x86::interrupts::interrupt_t vector, hype::cpu_registers_t& registers);
void start_handling_if_prompted();
void wait_for_server();

}
