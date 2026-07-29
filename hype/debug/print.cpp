
#include <x86/opcode_decode.h>
#include <efi/efi_base.h>
#include <base.h>

#include "print.h"

namespace hype::debug {

static bool isprint(const uint8_t c) {
    return c >= 0x20 && c <= 0x7E;
}

void ascii_format(char* buffer, size_t& offset, size_t& buffer_size, const char* fmt, ...) {
    VA_LIST args;
    VA_START(args, fmt);
    const auto written = AsciiVSPrint(buffer + offset, buffer_size, fmt, args);
    VA_END(args);

    offset += written;
    buffer_size -= written;
}

void memdump(const void* data, const size_t length) {
    char buffer[512];
    size_t offset = 0;
    size_t buffer_size = sizeof(buffer);

    const auto* ptr = static_cast<const uint8_t*>(data);
    ascii_format(buffer, offset, buffer_size, "Dump 0x%p -> 0x%p:\n", ptr, ptr + length);

    for (int i = 0; i < length; i += 16) {
        ascii_format(buffer, offset, buffer_size, "%04x ", ptr + i);

        for (int j = 0; j < 16; ++j) {
            const auto b = ptr[i + j];
            ascii_format(buffer, offset, buffer_size, "%02x ", b);
        }

        ascii_format(buffer, offset, buffer_size, " ");

        for (int j = 0; j < 16; ++j) {
            const auto b = ptr[i + j];
            ascii_format(buffer, offset, buffer_size, "%c", isprint(b) ? b : '.');
        }

        ascii_format(buffer, offset, buffer_size, "\n");
    }

    buffer[offset] = '\0';
    trace_debug("%a", buffer);
}

static void print_signed_hex(char* buffer, size_t& offset, size_t& buffer_size, const int64_t val) {
    ascii_format(buffer, offset, buffer_size, "%a0x%lx", (val < 0) ? "-" : "", (val < 0) ? -val : val);
}

static void print_op(char* buffer, size_t& offset, size_t& buffer_size, const x86::opcode::decoded_operand_t& operand) {
    if (operand.type == x86::opcode::decoded_operand_type_t::none) {
        return;
    }
    switch (operand.type) {
        case x86::opcode::decoded_operand_type_t::immediate_byte:
            ascii_format(buffer, offset, buffer_size, "0x%x", operand.value.i_byte);
            break;
        case x86::opcode::decoded_operand_type_t::immediate_word:
            ascii_format(buffer, offset, buffer_size, "0x%x", operand.value.i_word);
            break;
        case x86::opcode::decoded_operand_type_t::immediate_dword:
            ascii_format(buffer, offset, buffer_size, "0x%x", operand.value.i_dword);
            break;
        case x86::opcode::decoded_operand_type_t::immediate_qword:
            ascii_format(buffer, offset, buffer_size, "0x%llx", operand.value.i_qword);
            break;
        case x86::opcode::decoded_operand_type_t::instruction_displacement: {
            ascii_format(buffer, offset, buffer_size, "rel ");
            print_signed_hex(buffer, offset, buffer_size, operand.value.instruct_displacement);
            break;
            break;
        }
        case x86::opcode::decoded_operand_type_t::reg:
            ascii_format(buffer, offset, buffer_size, "%a", x86::opcode::get_register_name(operand.value.reg));
            break;
        case x86::opcode::decoded_operand_type_t::memory:
            ascii_format(buffer, offset, buffer_size, "[%a + ",
                x86::opcode::get_register_name(operand.value.mem.base));
            print_signed_hex(buffer, offset, buffer_size, operand.value.mem.displacement);
            ascii_format(buffer, offset, buffer_size, "]");
            break;
        case x86::opcode::decoded_operand_type_t::memory_offset:
            ascii_format(buffer, offset, buffer_size, "[");
            print_signed_hex(buffer, offset, buffer_size, operand.value.mem_offset.displacement);
            ascii_format(buffer, offset, buffer_size, "]");
            break;
        case x86::opcode::decoded_operand_type_t::memory_scaled:
            ascii_format(buffer, offset, buffer_size, "[%a + (%a * %ld) + ",
                x86::opcode::get_register_name(operand.value.mem_scaled.base),
                x86::opcode::get_register_name(operand.value.mem_scaled.index),
                operand.value.mem_scaled.scale);
            print_signed_hex(buffer, offset, buffer_size, operand.value.mem_offset.displacement);
            ascii_format(buffer, offset, buffer_size, "]");
            break;
        case x86::opcode::decoded_operand_type_t::memory_scaled2:
            ascii_format(buffer, offset, buffer_size, "[(%a * %ld) + ",
                x86::opcode::get_register_name(operand.value.mem_scaled2.index),
                operand.value.mem_scaled2.scale);
            print_signed_hex(buffer, offset, buffer_size, operand.value.mem_scaled2.displacement);
            ascii_format(buffer, offset, buffer_size, "]");
            break;
        case x86::opcode::decoded_operand_type_t::memory_sum:
            ascii_format(buffer, offset, buffer_size, "[%a + %a + ",
                x86::opcode::get_register_name(operand.value.mem_sum.reg1),
                x86::opcode::get_register_name(operand.value.mem_sum.reg2));
            print_signed_hex(buffer, offset, buffer_size, operand.value.mem_sum.displacement);
            ascii_format(buffer, offset, buffer_size, "]");
            break;
        default:
            break;
    }
}

static void print_opcode(char* buffer, size_t& offset, size_t& buffer_size, const uint8_t* opcode_ptr, const size_t opcode_size, const x86::opcode::decoded_opcode_t& opcode) {
    ascii_format(buffer, offset, buffer_size, "\n");
    ascii_format(buffer, offset, buffer_size, "%04x ", opcode_ptr);

    for (int i = 0; i < opcode_size; ++i) {
        const auto b = opcode_ptr[i];
        ascii_format(buffer, offset, buffer_size, "%02x ", b);
    }

    ascii_format(buffer, offset, buffer_size, "\t");

    ascii_format(buffer, offset, buffer_size, "%a ", x86::opcode::get_instruction_mnemonic(opcode.instruction));
    print_op(buffer, offset, buffer_size, opcode.op1);
    if (opcode.op1.type != x86::opcode::decoded_operand_type_t::none && opcode.op2.type != x86::opcode::decoded_operand_type_t::none) {
        ascii_format(buffer, offset, buffer_size, ", ");
    }
    print_op(buffer, offset, buffer_size, opcode.op2);
}

void instruction_dump(const void* data, size_t count) {
    char buffer[512];
    size_t offset = 0;
    size_t buffer_size = sizeof(buffer);

    ascii_format(buffer, offset, buffer_size, "Instruction Dump:");

    const auto* ptr = static_cast<const uint8_t*>(data);
    while ((count--) > 0) {
        const auto res = x86::opcode::decode(x86::opcode::mode_t::long_mode, ptr);
        if (res.success) {
            print_opcode(buffer, offset, buffer_size, ptr, (static_cast<const uint8_t*>(res.ptr) - ptr), res.opcode);
            ptr = static_cast<const uint8_t*>(res.ptr);
        } else {
            ascii_format(buffer, offset, buffer_size, "\nFailed to decode: code=0x%x", res.error_code);
            break;
        }
    }

    buffer[offset] = '\0';
    trace_debug("%a", buffer);
}

void print_page_mapping_simple(const x86::cr3_t& cr3, const x86::paging::ia32e::linear_address_t address) {
    char buffer[512];
    size_t offset = 0;
    size_t buffer_size = sizeof(buffer);

    ascii_format(buffer, offset, buffer_size, "GVA->GPA: ");

    using namespace x86::paging::ia32e;
    const auto pml4_address = static_cast<physical_address_t>(cr3.ia32e.address) << x86::paging::page_bits_4k;
    const auto pml4 = reinterpret_cast<const pml4e_t*>(pml4_address);
    const auto& pml4e = pml4[address.huge.pml4e];
    if (pml4e.bits.present) {
        ascii_format(buffer, offset, buffer_size, "PML4E to PDPT at 0x%llx [rw:%d,xd:%d]; ", pml4e.address(), pml4e.bits.rw, pml4e.bits.xd);
        const auto pdpt_address = pml4e.address();
        const auto* pdpt = reinterpret_cast<const pdpte_t*>(pdpt_address);
        const auto& pdpte = pdpt[address.huge.directory_pointer];
        if (pdpte.huge.present) {
            if (pdpte.is_huge()) {
                ascii_format(buffer, offset, buffer_size, "PDPTE maps as huge to 0x%llx [rw:%d,xd:%d]; ", pdpte.address(), pdpte.huge.rw, pdpte.huge.xd);
            } else {
                ascii_format(buffer, offset, buffer_size, "PDPTE to PD at 0x%llx [rw:%d,xd:%d]; ", pdpte.address(), pdpte.small.rw, pdpte.small.xd);
                const auto pd_address = pdpte.address();
                const auto* pd = reinterpret_cast<const pde_t*>(pd_address);
                const auto& pde = pd[address.large.directory];
                if (pde.large.present) {
                    if (pde.is_large()) {
                        ascii_format(buffer, offset, buffer_size, "PDE maps as large to 0x%llx [rw:%d,xd:%d]; ", pde.address(), pde.large.rw, pde.large.xd);
                    } else {
                        ascii_format(buffer, offset, buffer_size, "PDE to PT at 0x%llx [rw:%d,xd:%d]; ", pde.address(), pde.small.rw, pde.small.xd);
                        const auto pt_address = pde.address();
                        const auto* pt = reinterpret_cast<const pte_t*>(pt_address);
                        const auto& pte = pt[address.small.table];
                        if (pte.bits.present) {
                            ascii_format(buffer, offset, buffer_size, "PTE maps to 0x%llx [rw:%d,xd:%d]; ", pte.address(), pte.bits.rw, pte.bits.xd);
                        } else {
                            ascii_format(buffer, offset, buffer_size, "PTE not present; ");
                        }
                    }
                } else {
                    ascii_format(buffer, offset, buffer_size, "PDE not present; ");
                }
            }
        } else {
            ascii_format(buffer, offset, buffer_size, "PDPTE not present; ");
        }
    } else {
        ascii_format(buffer, offset, buffer_size, "PML4E not present; ");
    }

    buffer[offset] = '\0';
    trace_debug("%a", buffer);
}

void print_ept_mapping_simple(const x86::vmx::ept_pointer_t& eptp, const x86::vmx::guest_physical_address_t address) {
    char buffer[512];
    size_t offset = 0;
    size_t buffer_size = sizeof(buffer);

    ascii_format(buffer, offset, buffer_size, "GVA->HPA: ");

    using namespace x86::vmx;
    const auto pml4_address = static_cast<physical_address_t>(eptp.bits.address) << x86::paging::page_bits_4k;
    const auto pml4 = reinterpret_cast<const pml4e_t*>(pml4_address);
    auto& pml4e = pml4[address.huge.pml4e];
    if (pml4e.present()) {
        ascii_format(buffer, offset, buffer_size, "PML4E to PDPT at 0x%llx [r:%d,w:%d,x:%d]; ", pml4e.address(), pml4e.bits.read, pml4e.bits.write, pml4e.bits.execute);
        const auto pdpt_address = pml4e.address();
        const auto* pdpt = reinterpret_cast<const pdpte_t*>(pdpt_address);
        const auto& pdpte = pdpt[address.huge.directory_pointer];
        if (pdpte.present()) {
            if (pdpte.is_huge()) {
                ascii_format(buffer, offset, buffer_size, "PDPTE maps as huge to 0x%llx [r:%d,w:%d,x:%d]; ", pdpte.address(), pdpte.huge.read, pdpte.huge.write, pdpte.huge.execute);
            } else {
                ascii_format(buffer, offset, buffer_size, "PDPTE to PD at 0x%llx [r:%d,w:%d,x:%d]; ", pdpte.address(), pdpte.small.read, pdpte.small.write, pdpte.small.execute);
                const auto pd_address = pdpte.address();
                const auto* pd = reinterpret_cast<const pde_t*>(pd_address);
                const auto& pde = pd[address.large.directory];
                if (pde.present()) {
                    if (pde.is_large()) {
                        ascii_format(buffer, offset, buffer_size, "PDE maps as large to 0x%llx [r:%d,w:%d,x:%d]; ", pde.address(), pde.large.read, pde.large.write, pde.large.execute);
                    } else {
                        ascii_format(buffer, offset, buffer_size, "PDE to PT at 0x%llx [r:%d,w:%d,x:%d]; ", pde.address(), pde.small.read, pde.small.write, pde.small.execute);
                        const auto pt_address = pde.address();
                        const auto* pt = reinterpret_cast<const pte_t*>(pt_address);
                        const auto& pte = pt[address.small.table];
                        if (pte.present()) {
                            ascii_format(buffer, offset, buffer_size, "PTE maps to 0x%llx [r:%d,w:%d,x:%d]; ", pte.address(), pte.bits.read, pte.bits.write, pte.bits.execute);
                        } else {
                            ascii_format(buffer, offset, buffer_size, "PTE not present; ");
                        }
                    }
                } else {
                    ascii_format(buffer, offset, buffer_size, "PDE not present; ");
                }
            }
        } else {
            ascii_format(buffer, offset, buffer_size, "PDPTE not present; ");
        }
    } else {
        ascii_format(buffer, offset, buffer_size, "PML4E not present; ");
    }

    buffer[offset] = '\0';
    trace_debug("%a", buffer);
}

}
