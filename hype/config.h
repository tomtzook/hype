#pragma once

namespace hype::config {

constexpr bool print_guest_stack_on_vmexit = false;
constexpr bool decode_guest_instructions_on_vmexit = false;
constexpr bool do_vmentry_checks = false;
constexpr bool embedded_gdbstub = false;
constexpr bool wait_for_gdb_connect = false;

}
