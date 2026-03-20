
#include "../context.h"
#include "x86/types.h"

namespace gdbstub {

static const char hex_chars[] = "0123456789abcdef";

struct register_def_t {
    size_t size;
    size_t offset;
    uint64_t value;
};

enum class register_t {
    // order is important
    rax = 0,
    rbx,
    rcx,
    rdx,
    rsi,
    rdi,
    rbp,
    rsp,
    r8,
    r9,
    r10,
    r11,
    r12,
    r13,
    r14,
    r15,
    rip,
    rflags,
    cs,
    ss,
    ds,
    es,
    fs,
    gs,

    registers_last
};

enum {
    command_status = '?',
    command_get_registers = 'g',
    command_set_registers = 'G',
    command_read_memory = 'm',
    command_write_memory = 'M',
    command_get_register = 'p',
    command_set_register = 'P',
    command_query = 'q',
    command_set_breakpoint = 'Z',
    command_clear_breakpoint = 'z',
    command_continue = 'c',
    command_single_step = 's',
};

static constexpr size_t registers_count = static_cast<size_t>(register_t::registers_last);
register_def_t registers[registers_count] {
    {8, __builtin_offsetof(hype::cpu_registers_t, rax)},
    {8, __builtin_offsetof(hype::cpu_registers_t, rbx)},
    {8, __builtin_offsetof(hype::cpu_registers_t, rcx)},
    {8, __builtin_offsetof(hype::cpu_registers_t, rdx)},
    {8, __builtin_offsetof(hype::cpu_registers_t, rsi)},
    {8, __builtin_offsetof(hype::cpu_registers_t, rdi)},
    {8, __builtin_offsetof(hype::cpu_registers_t, rbp)},
    {8, __builtin_offsetof(hype::cpu_registers_t, rsp)},
    {8, __builtin_offsetof(hype::cpu_registers_t, r8)},
    {8, __builtin_offsetof(hype::cpu_registers_t, r9)},
    {8, __builtin_offsetof(hype::cpu_registers_t, r10)},
    {8, __builtin_offsetof(hype::cpu_registers_t, r11)},
    {8, __builtin_offsetof(hype::cpu_registers_t, r12)},
    {8, __builtin_offsetof(hype::cpu_registers_t, r13)},
    {8, __builtin_offsetof(hype::cpu_registers_t, r14)},
    {8, __builtin_offsetof(hype::cpu_registers_t, r15)},
    {8, __builtin_offsetof(hype::cpu_registers_t, rip)},
    {8, __builtin_offsetof(hype::cpu_registers_t, rflags)},
    {2, __builtin_offsetof(hype::cpu_registers_t, cs)},
    {2, __builtin_offsetof(hype::cpu_registers_t, ss)},
    {2, __builtin_offsetof(hype::cpu_registers_t, ds)},
    {2, __builtin_offsetof(hype::cpu_registers_t, es)},
    {2, __builtin_offsetof(hype::cpu_registers_t, fs)},
    {2, __builtin_offsetof(hype::cpu_registers_t, gs)},
};

static constexpr size_t stack_size = 10000;
uint8_t remcom_stack[stack_size];
uint8_t* stack_ptr = remcom_stack + stack_size;

static constexpr size_t buffer_size = 400;
static char remcon_in_buffer[buffer_size];
static char remcon_out_buffer[buffer_size];

static void (*volatile mem_fault_routine) ();
static volatile int mem_err = 0;

extern char read_next_char();
extern void write_next_char(char ch);

void set_mem_err() {
    mem_err = 1;
}

static void store_register(const register_t reg, const hype::cpu_registers_t& regs) {
    auto& def = registers[static_cast<int>(reg)];
    switch (def.size) {
        case 2: {
            uint16_t value;
            memcpy(&value, reinterpret_cast<const uint8_t*>(&regs) + def.offset, 2);
            def.value = value;
            break;
        }
        case 4: {
            uint32_t value;
            memcpy(&value, reinterpret_cast<const uint8_t*>(&regs) + def.offset, 4);
            def.value = value;
            break;
        }
        case 8: {
            uint64_t value;
            memcpy(&value, reinterpret_cast<const uint8_t*>(&regs) + def.offset, 8);
            def.value = value;
            break;
        }
        default:
            break;
    }
}

static void load_register(const register_t reg, hype::cpu_registers_t& regs) {
    const auto& def = registers[static_cast<int>(reg)];
    switch (def.size) {
        case 2: {
            const uint16_t value = def.value;
            memcpy(reinterpret_cast<uint8_t*>(&regs) + def.offset, &value, 2);
            break;
        }
        case 4: {
            const uint32_t value = def.value;
            memcpy(reinterpret_cast<uint8_t*>(&regs) + def.offset, &value, 4);
            break;
        }
        case 8: {
            const uint64_t value = def.value;
            memcpy(reinterpret_cast<uint8_t*>(&regs) + def.offset, &value, 8);
            break;
        }
        default:
            break;
    }
}

static void store_registers(const hype::cpu_registers_t& regs) {
    for (int i = 0; i < registers_count; i++) {
        store_register(static_cast<register_t>(i), regs);
    }
}

static void load_registers(hype::cpu_registers_t& regs) {
    for (int i = 0; i < registers_count; i++) {
        load_register(static_cast<register_t>(i), regs);
    }
}

static char hex(const char ch) {
    if ((ch >= 'a') && (ch <= 'f')) {
        return static_cast<char>(ch - 'a' + 10);
    }
    if ((ch >= '0') && (ch <= '9')) {
        return static_cast<char>(ch - '0');
    }
    if ((ch >= 'A') && (ch <= 'F')) {
        return static_cast<char>(ch - 'A' + 10);
    }

    return -1;
}

static char* mem2hex(const char* mem, char* buffer, const int count, const bool may_fault) {
    if (may_fault) {
        mem_fault_routine = set_mem_err;
    }

    for (int i = 0; i < count; i++) {
        const auto ch = mem[i];
        if (may_fault && mem_err) {
            return buffer;
        }

        *buffer++ = hex_chars[ch >> 4];
        *buffer++ = hex_chars[ch % 16];
    }

    *buffer = '\0';
    if (may_fault) {
        mem_fault_routine = nullptr;
    }

    return buffer;
}

static char* hex2mem(const char* buffer, char* mem, const int count, const bool may_fault) {
    if (may_fault) {
        mem_fault_routine = set_mem_err;
    }

    for (int i = 0; i < count; i++) {
        unsigned char ch = hex(*buffer++) << 4;
        ch += hex(*buffer++);

        mem[i] = static_cast<char>(ch);
        if (may_fault && mem_err) {
            return mem;
        }
    }

    if (may_fault) {
        mem_fault_routine = nullptr;
    }

    return mem;
}

static int hex2int(char** ptr, int* value_out) {
    int num_chars = 0;
    int value = 0;

    while (**ptr) {
        const auto hex_value = hex(**ptr);
        if (hex_value >= 0) {
            value = (value << 4) | hex_value;
            num_chars++;
        } else {
            break;
        }

        (*ptr)++;
    }

    *value_out = value;
    return num_chars;
}

static char hex_byte_low(const uint8_t val) {
    return hex_chars[val & 0x0f];
}

static char hex_byte_high(const uint8_t val) {
    return hex_chars[(val & 0xf0) >> 4];
}

static char* pack_hex(char* ptr, const uint8_t val) {
    *ptr++ = hex_byte_high(val);
    *ptr++ = hex_byte_low(val);
    return ptr;
}

static void read_to_packet_start() {
    char ch;
    do {
        ch = read_next_char();
    } while (ch != '$');
}

static int read_packet_data(char* buffer, const int max_size, int& checksum_out) {
    int checksum = 0;
    int count = 0;

    // read buffer data
    while (count < max_size - 1) {
        const auto ch = read_next_char();
        if (ch == '$') {
            // we got to a start again, start again
            return -1;
        }
        if (ch == '#') {
            // end of packet
            break;
        }

        checksum += ch;
        buffer[count++] = ch;
    }

    checksum_out = checksum;
    return count;
}

static char* read_packet(char* buffer, const int max_size) {
    while (true) {
        read_to_packet_start();

        int checksum = 0;
        int count = 0;

        do {
            count = read_packet_data(buffer, max_size, checksum);
        } while (count < 0);

        const auto last_ch = buffer[count-1];
        buffer[count] = '\0';
        if (last_ch == '#') {
            // we've found end of packet

            auto ch = read_next_char();
            int xmitcsum = hex(ch) << 4;
            ch = read_next_char();
            xmitcsum += hex(ch);

            if (checksum != xmitcsum) {
                // bad message
                // todo: trace
                write_next_char('-');
            } else {
                // good message
                write_next_char('+');

                // if a sequence char is present, reply the sequence ID
                if (buffer[2] == ':') {
                    write_next_char(buffer[0]);
                    write_next_char(buffer[1]);

                    // start of data is after the id
                    return &buffer[3];
                }
            }

            return &buffer[1];
        }
    }
}

static void write_packet(const char* buffer) {
    do {
        write_next_char('$');
        int checksum = 0;
        int count = 0;

        char ch;
        while ((ch = buffer[count])) {
            write_next_char(ch);
            checksum += ch;
            count++;
        }

        write_next_char('#');
        write_next_char(hex_byte_high(checksum));
        write_next_char(hex_byte_low(checksum));
    } while (read_next_char() != '+');
}

static void write_exception_occurred(const int signal) {
    {
        auto* ptr = remcon_out_buffer;
        *ptr++ = 'T';
        ptr = pack_hex(ptr, signal);
        *ptr++ = ';';
        *ptr = '\0';
    }

    write_packet(remcon_out_buffer);
}

void handle_exception(const int vector) {
    const auto sigval = vector2signal(vector);
    write_exception_occurred(sigval);

    while (true) {
        auto ptr = read_packet(remcon_in_buffer, buffer_size);
        const auto command = *ptr++;
        switch (command) {

        }
    }
}

}
