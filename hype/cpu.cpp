
#include "cpu.h"

namespace hype {

void deadloop() {
    //trace_debug("Entering Deadloop");

    static volatile int wait = 1;
    while (wait) {
        __asm__ __volatile__("pause");
    }

    //trace_debug("Leaving Deadloop");
}

void hlt_cpu() {
    __asm__ volatile ("cli; hlt");
}

}
