#pragma once

#include <base.h>

extern "C" void asm_vm_entry();
extern "C" void asm_vm_resume();

namespace hype {

framework::result<> do_vm_entry_checks();

}
