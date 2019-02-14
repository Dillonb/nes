#include <stdlib.h>
#include <err.h>

#include "cpu.h"
#include "mem.h"
#include "debugger.h"
#include "util.h"
#include "opcode_names.h"

const char* docs_prefix = "https://www.masswerk.at/6502/6502_instruction_set.html#";
#define DOCS_PREFIX_LENGTH 55

#define NMI_PC_LOCATION 0xFFFA

long total_cycles = 0;
int stall_cycles = 0;

void stall_cpu(int cycles) {
    stall_cycles += cycles;
}

long get_total_cpu_cycles() {
    return total_cycles;
}

byte read_byte_and_inc_pc(memory* mem) {
    byte data = read_byte(mem, mem->pc);
    mem->pc++;
    return data;
}

uint16_t read_address(memory* mem, uint16_t address) {
    uint16_t addr_upper = address & (uint16_t)0xFF00;
    byte     addr_lower = (byte)(address & 0x00FF) + (byte)1;

    uint16_t lower_byte_address = address;
    uint16_t upper_byte_address = addr_upper | addr_lower;

    byte lower = read_byte(mem, lower_byte_address);
    byte upper = read_byte(mem, upper_byte_address);

    return ((uint16_t)upper << 8) | lower;
}

uint16_t read_address_and_inc_pc(memory* mem) {
    byte lower = read_byte_and_inc_pc(mem);
    byte upper = read_byte_and_inc_pc(mem);
    return (upper << 8) | lower;
}

uint16_t indirect_x_address(memory* mem, int* cycles) {
    byte b = read_byte_and_inc_pc(mem);
    byte temp_addr = b + mem->x;
    uint16_t addr = read_address(mem, temp_addr);
    return addr;
}


uint16_t indirect_y_address(memory* mem, int* cycles) {
    byte b = read_byte_and_inc_pc(mem);
    uint16_t addr = read_address(mem, b);
    dprintf("Ind Y: b: 0x%02X ind addr: 0x%04x y: 0x%02X\n", b, addr, mem->y);
    if (cycles != NULL) {
        *cycles += (0xFF & addr) > (0xFF & addr + mem->y); // If page crossed, add a cycle
    }
    addr += mem->y;

    return addr;
}

uint16_t absolute_x_address(memory* mem, int* cycles) {
    uint16_t addr = read_address_and_inc_pc(mem);
    if (cycles != NULL) {
        *cycles += (0xFF & addr) > (0xFF & addr + mem->x); // If page crossed, add a cycle
    }
    addr += mem->x;
    return addr;
}

uint16_t absolute_y_address(memory* mem, int* cycles) {
    uint16_t addr = read_address_and_inc_pc(mem);
    if (cycles != NULL) {
        *cycles += (0xFF & addr) > (0xFF & addr + mem->y); // If page crossed, add a cycle
    }
    addr += mem->y;
    return addr;
}

uint16_t zeropage_x_address(memory* mem) {
    return ((uint16_t)read_byte_and_inc_pc(mem) + (uint16_t)mem->x) & (uint16_t)0xFF;
}

uint16_t zeropage_y_address(memory* mem) {
    return ((uint16_t)read_byte_and_inc_pc(mem) + (uint16_t)mem->y) & (uint16_t)0xFF;
}

byte read_value(memory* mem, int* cycles, addressing_mode mode) {
    switch (mode) {
        case Immediate:
            return read_byte_and_inc_pc(mem);

        case Absolute:
            return read_byte(mem, read_address_and_inc_pc(mem));

        case Absolute_X: {
            return read_byte(mem, absolute_x_address(mem, cycles));
        }

        case Absolute_Y: {
            return read_byte(mem, absolute_y_address(mem, cycles));
        }

        case Indirect_X: {
            return read_byte(mem, indirect_x_address(mem, cycles));
        }

        case Indirect_Y: {
            return read_byte(mem, indirect_y_address(mem, cycles));
        }

        case Zeropage: {
            return read_byte(mem, read_byte_and_inc_pc(mem));
        }

        case Zeropage_X: {
            return read_byte(mem, zeropage_x_address(mem));
        }

        case Zeropage_Y: {
            return read_byte(mem, zeropage_y_address(mem));
        }

        default:
            errx(EXIT_FAILURE, "read_value: Addressing mode not implemented.");
    }
}

uint16_t address_for_opcode(memory* mem, byte opcode, int* cycles) {
    addressing_mode mode = opcode_addressing_modes[opcode];
    switch (mode) {
        case Immediate:
            errx(EXIT_FAILURE, "The Immediate addressing mode is not supported in address_for_opcode() !");

        case Absolute: {
            return read_address_and_inc_pc(mem);
        }

        case Absolute_X: {
            return absolute_x_address(mem, cycles);
        }

        case Absolute_Y: {
            return absolute_y_address(mem, cycles);
        }

        case Indirect:
            return read_address(mem, read_address_and_inc_pc(mem));

        case Indirect_X:
            return indirect_x_address(mem, cycles);

        case Indirect_Y: {
            return indirect_y_address(mem, cycles);
        }

        case Zeropage: {
            return read_byte_and_inc_pc(mem);
        }

        case Zeropage_X: {
            return zeropage_x_address(mem);
        }

        case Zeropage_Y: {
            return zeropage_y_address(mem);
        }

        default:
            errx(EXIT_FAILURE, "address_for_opcode: Addressing mode not implemented.");
    }
}

byte value_for_opcode(memory* mem, byte opcode, int* cycles) {
    return read_value(mem, cycles, opcode_addressing_modes[opcode]);
}

void branch_on_condition(memory* mem, int* cycles, bool condition) {
    // Read as a signed byte
    int8_t offset = read_byte_and_inc_pc(mem);
    if (condition) {
        uint16_t newaddr = mem->pc + offset;
        dprintf("Branch condition hit, branching! offset: %d (0x%x) newaddr: 0x%x\n", offset, offset, newaddr);
        *cycles += SAME_PAGE(mem->pc, newaddr) ? 1 : 2;
        mem->pc = newaddr;
    }
    else {
            dprintf("Did not hit branch condition, not branching!\n");
    }
}

void cmp(memory* mem, byte reg, byte value) {
    byte result = reg - value;
    set_p_zn_on(mem, result);
    if (reg >= value) {
        set_p_carry(mem);
    }
    else {
        clear_p_carry(mem);
    }
}

void php(memory* mem) {
    byte temp_p = mem->p;
    temp_p |= 0b00110000;
    stack_push(mem, temp_p);
}

void adc(memory* mem, byte value) {
    byte old_a = mem->a;
    byte carry = (byte)get_p_carry(mem);

    mem->a = old_a + carry + value;

    if ((uint16_t)old_a + (uint16_t)carry + (uint16_t)value > 0x00FF) {
        set_p_carry(mem);
    }
    else {
        clear_p_carry(mem);
    }

    set_p_zn_on(mem, mem->a);
    bool old_a_and_value_same_signs = (((old_a^value)&0x80) == 0);
    bool old_a_and_new_a_different_signs =  (((old_a^mem->a)&0x80) != 0);
    set_p_overflow_to(mem, old_a_and_value_same_signs && old_a_and_new_a_different_signs);

}

void sbc(memory* mem, byte value) {
    byte old_a = mem->a;
    bool carry = get_p_carry(mem);
    mem->a = old_a - value - (1 - carry);
    set_p_zn_on(mem, mem->a);
    set_p_carry_to(mem, (int)old_a - (int)value - (int)(1 - carry) >= 0);
    set_p_overflow_to(mem, ((old_a ^ value) & 0x80) != 0 && ((old_a ^ mem->a) & 0x80) != 0);
}

int interrupt_nmi(memory* mem) {
    stack_push16(mem, mem->pc);
    php(mem);
    mem->pc = read_address(mem, NMI_PC_LOCATION);
    dprintf("Read and jumped to address 0x%04x out of 0x%04x for NMI handler\n", mem->pc, NMI_PC_LOCATION);
    set_p_interrupt(mem);
    return 7;
}

interrupt_type interrupt = NONE;

int interrupt_cpu_step(memory* mem) {
    debug_hook(INTERRUPT, mem);
    // Before doing the step, see if there was an interrupt triggered
    if (interrupt == nmi) {
        return interrupt_nmi(mem);
    }
    errx(EXIT_FAILURE, "Interrupt type not implemented");
}

void ror(memory* mem, uint16_t address) {
    bool oldc = (bool) get_p_carry(mem);
    byte value = read_byte(mem, address);
    set_p_carry_to(mem, (bool) (value & 1));
    value = (byte) (((value >> 1) & 0b01111111) | ((byte)oldc << 7));
    write_byte(mem, address, value);
}

void rol(memory* mem, uint16_t address) {
    bool oldc = (bool) get_p_carry(mem);
    byte value = read_byte(mem, address);
    set_p_carry_to(mem, (value >> 7) & 1);
    value = (value << 1) | oldc;
    set_p_zn_on(mem, value);
    write_byte(mem, address, value);
}

int normal_cpu_step(memory* mem) {
    debug_hook(STEP, mem);
    uint16_t old_pc = mem->pc;
    byte opcode = read_byte_and_inc_pc(mem);
    int cycles = opcode_cycles[opcode];

    switch (opcode) {
        case BRK: {
            // Set interrupt flag on status register
            set_p_interrupt(mem);
            mem->pc++; // Skip next byte
            break;
        }

        case SEI: {
            set_p_interrupt(mem);
            break;
        }

        case STA_Zeropage:
        case STA_Zeropage_X:
        case STA_Absolute:
        case STA_Absolute_X:
        case STA_Absolute_Y:
        case STA_Indirect_X:
        case STA_Indirect_Y: {
            write_byte(mem, address_for_opcode(mem, opcode, NULL), mem->a);
            break;
        }

        case STX_Absolute:
        case STX_Zeropage:
        case STX_Zeropage_Y: {
            write_byte(mem, address_for_opcode(mem, opcode, NULL), mem->x);
            break;
        }

        case STY_Absolute:
        case STY_Zeropage:
        case STY_Zeropage_X: {
            write_byte(mem, address_for_opcode(mem, opcode, NULL), mem->y);
            break;
        }

        case TXS: {
            mem->sp = mem->x;
            break;
        }

        case TXA: {
            mem->a = mem->x;
            set_p_zn_on(mem, mem->a);
            break;
        }

        case TAX: {
            mem->x = mem->a;
            set_p_zn_on(mem, mem->x);
            break;
        }

        case TAY: {
            mem->y = mem->a;
            set_p_zn_on(mem, mem->y);
            break;
        }

        case TYA: {
            mem->a = mem->y;
            set_p_zn_on(mem, mem->a);
            break;
        }

        case TSX: {
            mem->x = mem->sp;
            set_p_zn_on(mem, mem->x);
            break;
        }

        case LDX_Immediate:
        case LDX_Zeropage:
        case LDX_Zeropage_Y:
        case LDX_Absolute:
        case LDX_Absolute_Y: {
            mem->x = value_for_opcode(mem, opcode, &cycles);
            set_p_zn_on(mem, mem->x);
            break;
        }

        case LDY_Immediate:
        case LDY_Zeropage:
        case LDY_Zeropage_X:
        case LDY_Absolute:
        case LDY_Absolute_X: {
            mem->y = value_for_opcode(mem, opcode, &cycles);
            set_p_zn_on(mem, mem->y);
            break;
        }

        case LDA_Immediate:
        case LDA_Zeropage:
        case LDA_Zeropage_X:
        case LDA_Absolute:
        case LDA_Absolute_X:
        case LDA_Absolute_Y:
        case LDA_Indirect_X:
        case LDA_Indirect_Y: {
            mem->a = value_for_opcode(mem, opcode, &cycles);
            set_p_zn_on(mem, mem->a);
            break;
        }

        case CLD: {
            clear_p_decimal(mem);
            break;
        }

        case SED: {
            set_p_decimal(mem);
            break;
        }

        case CLI: {
            clear_p_interrupt(mem);
            break;
        }

        case CLV: {
            clear_p_overflow(mem);
            break;
        }

        case BPL: {
            branch_on_condition(mem, &cycles, get_p_negative(mem) == false);
            break;
        }

        case BCS: {
            branch_on_condition(mem, &cycles, get_p_carry(mem) == true);
            break;
        }

        case BNE: {
            branch_on_condition(mem, &cycles, get_p_zero(mem) == false);
            break;
        }

        case BEQ: {
            branch_on_condition(mem, &cycles, get_p_zero(mem) == true);
            break;
        }

        case BCC: {
            branch_on_condition(mem, &cycles, get_p_carry(mem) == false);
            break;
        }

        case BMI: {
            branch_on_condition(mem, &cycles, get_p_negative(mem) == true);
            break;
        }

        case BVS: {
            branch_on_condition(mem, &cycles, get_p_overflow(mem) == true);
            break;
        }

        case BVC: {
            branch_on_condition(mem, &cycles, get_p_overflow(mem) == false);
            break;
        }

        case CMP_Immediate:
        case CMP_Zeropage:
        case CMP_Zeropage_X:
        case CMP_Absolute:
        case CMP_Absolute_X:
        case CMP_Absolute_Y:
        case CMP_Indirect_X:
        case CMP_Indirect_Y: {
            cmp(mem, mem->a, value_for_opcode(mem, opcode, &cycles));
            break;
        }

        case CPY_Immediate:
        case CPY_Zeropage:
        case CPY_Absolute: {
            cmp(mem, mem->y, value_for_opcode(mem, opcode, &cycles));
            break;
        }

        case CPX_Immediate:
        case CPX_Zeropage:
        case CPX_Absolute: {
            cmp(mem, mem->x, value_for_opcode(mem, opcode, &cycles));
            break;
        }

        case JSR: {
            uint16_t addr = read_address_and_inc_pc(mem);
            stack_push16(mem, mem->pc - 1);
            mem->pc = addr;
            break;
        }

        case RTS: {
            mem->pc = stack_pop16(mem) + 1;
            break;
        }

        case RTI: {
            mem->p = stack_pop(mem);
            mem->p &= 0b11101111;
            mem->p |= 0b00100000;
            mem->pc = stack_pop16(mem);
            break;
        }

        case DEY: {
            mem->y--;
            set_p_zn_on(mem, mem->y);
            break;
        }

        case DEX: {
            mem->x--;
            set_p_zn_on(mem, mem->x);
            break;
        }

        case INY: {
            mem->y++;
            set_p_zn_on(mem, mem->y);
            break;
        }

        case INX: {
            mem->x++;
            set_p_zn_on(mem, mem->x);
            break;
        }

        case INC_Absolute:
        case INC_Absolute_X:
        case INC_Zeropage:
        case INC_Zeropage_X: {
            uint16_t addr = address_for_opcode(mem, opcode, &cycles);
            byte value = read_byte(mem, addr);
            value++;
            write_byte(mem, addr, value);
            set_p_zn_on(mem, value);
            break;
        }

        case DEC_Absolute:
        case DEC_Absolute_X:
        case DEC_Zeropage:
        case DEC_Zeropage_X: {
            uint16_t addr = address_for_opcode(mem, opcode, &cycles);
            byte value = read_byte(mem, addr);
            value--;
            write_byte(mem, addr, value);
            set_p_zn_on(mem, value);
            break;
        }

        case BIT_Absolute:
        case BIT_Zeropage: {
            byte operand = value_for_opcode(mem, opcode, &cycles);
            bool new_negative = (mask_flag(P_NEGATIVE) & operand) > 0;
            bool new_overflow = (mask_flag(P_OVERFLOW) & operand) > 0;
            set_p_negative_to(mem, new_negative);
            set_p_overflow_to(mem, new_overflow);
            set_p_zero_on(mem, operand & mem->a);
            break;
        }

        case ORA_Immediate:
        case ORA_Absolute:
        case ORA_Absolute_X:
        case ORA_Absolute_Y:
        case ORA_Indirect_X:
        case ORA_Indirect_Y:
        case ORA_Zeropage:
        case ORA_Zeropage_X: {
            mem->a |= value_for_opcode(mem, opcode, &cycles);
            set_p_zn_on(mem, mem->a);
            break;
        }

        case AND_Immediate:
        case AND_Absolute:
        case AND_Absolute_X:
        case AND_Absolute_Y:
        case AND_Indirect_X:
        case AND_Indirect_Y:
        case AND_Zeropage:
        case AND_Zeropage_X: {
            mem->a &= value_for_opcode(mem, opcode, &cycles);
            set_p_zn_on(mem, mem->a);
            break;
        }

        case JMP_Absolute:
        case JMP_Indirect: {
            mem->pc = address_for_opcode(mem, opcode, &cycles);
            break;
        }

        case PHP: {
            php(mem);
            break;
        }

        case PHA: {
            stack_push(mem, mem->a);
            break;
        }

        case PLA: {
            mem->a = stack_pop(mem);
            set_p_zn_on(mem, mem->a);
            break;
        }

        case LSR_Accumulator: {
            set_p_carry_to(mem, (bool) (mem->a & 1));
            mem->a >>= 1;
            mem->a &= 0b01111111;
            set_p_zn_on(mem, mem->a);
            break;
        }

        case LSR_Absolute:
        case LSR_Absolute_X:
        case LSR_Zeropage:
        case LSR_Zeropage_X: {
            uint16_t addr = address_for_opcode(mem, opcode, &cycles);
            byte value = read_byte(mem, addr);
            set_p_carry_to(mem, (bool) (value & 1));
            value >>= 1;
            set_p_zn_on(mem, value);
            write_byte(mem, addr, value);
            break;
        }

        case ROL_Accumulator: {
            bool oldc = get_p_carry(mem);
            set_p_carry_to(mem, (mem->a >> 7) & 1);
            mem->a = (mem->a << 1) | oldc;
            set_p_zn_on(mem, mem->a);
            break;
        }

        case ROL_Absolute:
        case ROL_Absolute_X:
        case ROL_Zeropage:
        case ROL_Zeropage_X: {
            uint16_t address = address_for_opcode(mem, opcode, &cycles);
            rol(mem, address);
            break;
        }

        case ROR_Absolute:
        case ROR_Absolute_X:
        case ROR_Zeropage:
        case ROR_Zeropage_X: {
            uint16_t address = address_for_opcode(mem, opcode, &cycles);
            ror(mem, address);
            set_p_zn_on(mem, read_byte(mem, address));
            break;
        }

        case ROR_Accumulator: {
            bool oldc = (bool) get_p_carry(mem);
            set_p_carry_to(mem, (bool) (mem->a & 1));
            mem->a = (byte) (((mem->a >> 1) & 0b01111111) | ((byte)oldc << 7));
            set_p_zn_on(mem, mem->a);
            break;
        }

        case SEC: {
            set_p_carry(mem);
            break;
        }

        case SBC_Immediate:
        case SBC_Absolute:
        case SBC_Absolute_X:
        case SBC_Absolute_Y:
        case SBC_Indirect_X:
        case SBC_Indirect_Y:
        case SBC_Zeropage:
        case SBC_Zeropage_X: {
            sbc(mem, value_for_opcode(mem, opcode, &cycles));
            break;
        }

        case EOR_Zeropage:
        case EOR_Immediate:
        case EOR_Absolute:
        case EOR_Absolute_X:
        case EOR_Absolute_Y:
        case EOR_Indirect_X:
        case EOR_Indirect_Y:
        case EOR_Zeropage_X: {
            mem->a ^= value_for_opcode(mem, opcode, &cycles);
            set_p_zn_on(mem, mem->a);
            break;
        }

        case CLC: {
            clear_p_carry(mem);
            break;
        }

        case ASL_Accumulator: {
            set_p_carry_to(mem, (mem->a >> 7) & 1);
            mem->a <<= 1;
            set_p_zn_on(mem, mem->a);
            break;
        }

        case ASL_Absolute:
        case ASL_Absolute_X:
        case ASL_Zeropage:
        case ASL_Zeropage_X: {
            uint16_t addr = address_for_opcode(mem, opcode, &cycles);
            byte value = read_byte(mem, addr);
            set_p_carry_to(mem, (value >> 7) & 1);
            value <<= 1;
            set_p_zn_on(mem, value);
            write_byte(mem, addr, value);
            break;
        }

        case ADC_Immediate:
        case ADC_Absolute:
        case ADC_Absolute_X:
        case ADC_Absolute_Y:
        case ADC_Indirect_X:
        case ADC_Indirect_Y:
        case ADC_Zeropage:
        case ADC_Zeropage_X: {
            adc(mem, value_for_opcode(mem, opcode, &cycles));
            break;
        }

        case PLP: {
            mem->p = stack_pop(mem);
            mem->p &= 0b11101111;
            mem->p |= 0b00100000;
            break;
        }

        case NOP: {
            break;
        }

        default: {
            const char* opcode_short = opcode_to_name_short(opcode);
            char docs_link[DOCS_PREFIX_LENGTH + 10];
            snprintf(docs_link, sizeof(docs_link), "%s%s", docs_prefix, opcode_short);
            errx(EXIT_FAILURE, "Opcode not implemented: %s hex 0x%x\nSee %s", opcode_to_name_full(opcode), opcode, docs_link);
        }
    }

    return cycles;
}

void trigger_nmi() {
    dprintf("!!! NMI TRIGGERED !!!\n");
    interrupt = nmi;
}

void trigger_oam_dma(memory* mem, uint16_t address) {
    for (uint16_t i = 0; i < 0xFF; i++) {
        write_oam_byte(&mem->ppu_mem, read_byte(mem, address + i));
    }

    if (total_cycles % 2 == 1) {
        stall_cpu(1);
    }

    stall_cpu(513);
}

int cpu_step(memory* mem) {
    int cycles;
    if (interrupt != NONE) {
        cycles = interrupt_cpu_step(mem);
        interrupt = NONE;
    }
    else {
        cycles = normal_cpu_step(mem);
    }
    cycles += stall_cycles;
    total_cycles += cycles;
    stall_cycles = 0;
    return cycles;
}
