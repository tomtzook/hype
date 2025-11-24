#pragma once

#include <base.h>

extern "C" void asm_vm_entry();

namespace hype {

framework::result<> do_vm_entry_checks();

}
