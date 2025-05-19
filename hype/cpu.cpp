
#include "cpu.h"

namespace hype {

void hlt_cpu() {
    __asm__ volatile ("cli; hlt");
}

}
