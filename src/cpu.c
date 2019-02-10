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
    byte lower = read_byte(mem, address);
    byte upper = read_byte(mem, address + 1);
    return (upper << 8) | lower;
}

uint16_t read_address_and_inc_pc(memory* mem) {
    byte lower = read_byte_and_inc_pc(mem);
    byte upper = read_byte_and_inc_pc(mem);
    return (upper << 8) | lower;
}


// TODO: handle pages crossed cases
uint16_t indirect_y_address(memory* mem, int* cycles) {
    byte b = read_byte_and_inc_pc(mem);
    uint16_t address = read_address(mem, b);
    dprintf("Ind Y: b: 0x%02X ind addr: 0x%04x y: 0x%02X\n", b, address, mem->y);
    address += mem->y;

    return address;
}

// TODO: handle pages crossed cases
uint16_t absolute_x_address(memory* mem, int* cycles) {
    uint16_t addr = read_address_and_inc_pc(mem);
    addr += mem->x;
    return addr;
}

// TODO: handle pages crossed cases
uint16_t absolute_y_address(memory* mem, int* cycles) {
    uint16_t addr = read_address_and_inc_pc(mem);
    addr += mem->x;
    return addr;
}

uint16_t zeropage_x_address(memory* mem) {
    return ((uint16_t)read_byte_and_inc_pc(mem) + (uint16_t)mem->x) & 0xFF;
}

// TODO: handle pages crossed cases
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

        case Indirect_Y: {
            return read_byte(mem, indirect_y_address(mem, cycles));
        }

        case Zeropage: {
            return read_byte(mem, read_byte_and_inc_pc(mem));
        }

        case Zeropage_X: {
            return read_byte(mem, zeropage_x_address(mem));
        }

        default:
            errx(EXIT_FAILURE, "Addressing mode not implemented.");
    }
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
    if (result >= 0) {
        set_p_carry(mem);
    }
    else {
        clear_p_carry(mem);
    }
}

void php(memory* mem) {
    stack_push(mem, mem->p);
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

int normal_cpu_step(memory* mem) {
    debug_hook(STEP, mem);
    uint16_t old_pc = mem->pc;
    byte opcode = read_byte_and_inc_pc(mem);
    int cycles = 0;

    switch (opcode) {
        case BRK: {
            // Set interrupt flag on status register
            set_p_interrupt(mem);
            mem->pc++; // Skip next byte
            cycles = 7;
            break;
        }

        case SEI: {
            set_p_interrupt(mem);
            cycles = 2;
            break;
        }

        case STA_Absolute: {
            write_byte(mem, read_address_and_inc_pc(mem), mem->a);
            cycles = 4;
            break;
        }

        case STA_Absolute_Y: {
            cycles = 5;
            write_byte(mem, absolute_y_address(mem, &cycles), mem->a);
            break;
        }

        case STA_Absolute_X: {
            cycles = 5;
            write_byte(mem, absolute_x_address(mem, &cycles), mem->a);
            break;
        }

        case STA_Zeropage: {
            cycles = 3;
            write_byte(mem, read_byte_and_inc_pc(mem), mem->a);
            break;
        }

        case STA_Zeropage_X: {
            cycles = 4;
            write_byte(mem, zeropage_x_address(mem), mem->a);
            break;
        }

        case STA_Indirect_Y: {
            cycles = 6;
            write_byte(mem, indirect_y_address(mem, &cycles), mem->a);
            break;
        }

        case STX_Zeropage: {
            cycles = 3;
            write_byte(mem, read_byte_and_inc_pc(mem), mem->x);
            break;
        }

        case STX_Absolute: {
            cycles = 4;
            write_byte(mem, read_address_and_inc_pc(mem), mem->x);
            break;
        }

        case STY_Absolute: {
            cycles = 4;
            write_byte(mem, read_address_and_inc_pc(mem), mem->y);
            break;
        }

        case STY_Zeropage: {
            cycles = 3;
            write_byte(mem, read_byte_and_inc_pc(mem), mem->y);
            break;
        }

        case TXS: {
            mem->sp = mem->x;
            cycles = 2;
            break;
        }

        case TXA: {
            cycles = 2;
            mem->a = mem->x;
            set_p_zn_on(mem, mem->a);
            break;
        }

        case TAX: {
            mem->x = mem->a;
            set_p_zn_on(mem, mem->x);
            cycles = 2;
            break;
        }

        case TAY: {
            mem->y = mem->a;
            set_p_zn_on(mem, mem->y);
            cycles = 2;
            break;
        }

        case TYA: {
            mem->a = mem->y;
            set_p_zn_on(mem, mem->a);
            cycles = 2;
            break;
        }

        case LDX_Immediate: {
            mem->x = read_value(mem, &cycles, Immediate);
            set_p_zn_on(mem, mem->x);
            cycles = 2;
            break;
        }

        case LDX_Absolute: {
            mem->x = read_value(mem, &cycles, Absolute);
            set_p_zn_on(mem, mem->x);
            cycles = 4;
            break;
        }

        case LDX_Absolute_Y: {
            mem->x = read_value(mem, &cycles, Absolute_Y);
            set_p_zn_on(mem, mem->x);
            cycles = 4;
            break;
        }

        case LDX_Zeropage: {
            mem->x = read_value(mem, &cycles, Zeropage);
            set_p_zn_on(mem, mem->x);
            cycles = 3;
            break;
        }

        case LDY_Immediate: {
            mem->y = read_value(mem, &cycles, Immediate);
            set_p_zn_on(mem, mem->y);
            cycles = 2;
            break;
        }

        case LDY_Absolute: {
            mem->y = read_value(mem, &cycles, Absolute);
            set_p_zn_on(mem, mem->y);
            cycles = 4;
            break;
        }

        case LDY_Zeropage: {
            mem->y = read_value(mem, &cycles, Zeropage);
            set_p_zn_on(mem, mem->y);
            cycles = 3;
            break;
        }

        case LDY_Zeropage_X: {
            mem->y = read_value(mem, &cycles, Zeropage_X);
            set_p_zn_on(mem, mem->y);
            cycles = 4;
            break;
        }

        case LDA_Immediate: {
            mem->a = read_value(mem, &cycles, Immediate);
            set_p_zn_on(mem, mem->a);
            cycles = 2;
            break;
        }

        case LDA_Absolute: {
            mem->a = read_value(mem, &cycles, Absolute);
            set_p_zn_on(mem, mem->a);
            cycles = 4;
            break;
        }

        case LDA_Absolute_X: {
            mem->a = read_value(mem, &cycles, Absolute_X);
            set_p_zn_on(mem, mem->a);
            cycles = 4;
            break;
        }

        case LDA_Absolute_Y: {
            mem->a = read_value(mem, &cycles, Absolute_Y);
            set_p_zn_on(mem, mem->a);
            cycles = 4;
            break;
        }

        case LDA_Indirect_Y: {
            cycles = 5;
            mem->a = read_value(mem, &cycles, Indirect_Y);
            set_p_zn_on(mem, mem->a);
            break;
        }

        case LDA_Zeropage: {
            cycles = 3;
            mem->a = read_value(mem, &cycles, Zeropage);
            set_p_zn_on(mem, mem->a);
            break;
        }

        case LDA_Zeropage_X: {
            cycles = 3;
            mem->a = read_value(mem, &cycles, Zeropage_X);
            set_p_zn_on(mem, mem->a);
            break;
        }

        case CLD: {
            clear_p_decimal(mem);
            cycles = 2;
            break;
        }

        case BPL: {
            cycles = 2;
            branch_on_condition(mem, &cycles, get_p_negative(mem) == false);
            break;
        }

        case BCS: {
            cycles = 2;
            branch_on_condition(mem, &cycles, get_p_carry(mem) == true);
            break;
        }

        case BNE: {
            cycles = 2;
            branch_on_condition(mem, &cycles, get_p_zero(mem) == false);
            break;
        }

        case BEQ: {
            cycles = 2;
            branch_on_condition(mem, &cycles, get_p_zero(mem) == true);
            break;
        }

        case BCC: {
            cycles = 2;
            branch_on_condition(mem, &cycles, get_p_carry(mem) == false);
            break;
        }

        case BMI: {
            cycles = 2;
            branch_on_condition(mem, &cycles, get_p_negative(mem) == true);
            break;
        }

        case CMP_Immediate: {
            cycles = 2;
            cmp(mem, mem->a, read_value(mem, &cycles, Immediate));
            break;
        }

        case CMP_Zeropage: {
            cycles = 3;
            cmp(mem, mem->a, read_value(mem, &cycles, Zeropage));
            break;
        }

        case CMP_Absolute_X: {
            cycles = 4;
            cmp(mem, mem->a, read_value(mem, &cycles, Absolute_X));
            break;
        }

        case CMP_Absolute_Y: {
            cycles = 4;
            cmp(mem, mem->a, read_value(mem, &cycles, Absolute_Y));
            break;
        }

        case CPY_Immediate: {
            cycles = 2;
            cmp(mem, mem->y, read_value(mem, &cycles, Immediate));
            break;
        }

        case CPY_Absolute: {
            cycles = 4;
            cmp(mem, mem->y, read_value(mem, &cycles, Absolute));
            break;
        }

        case CPX_Immediate: {
            cycles = 2;
            cmp(mem, mem->x, read_value(mem, &cycles, Immediate));
            break;
        }

        case JSR: {
            cycles = 6;
            uint16_t addr = read_address_and_inc_pc(mem);
            stack_push16(mem, mem->pc); // TODO: should this be mem->pc+1?
            mem->pc = addr;
            break;
        }

        case RTS: {
            cycles = 6;
            mem->pc = stack_pop16(mem);
            break;
        }

        case RTI: {
            cycles = 6;
            mem->p = stack_pop(mem);
            mem->pc = stack_pop16(mem);
            break;
        }

        case DEY: {
            cycles = 2;
            mem->y--;
            set_p_zn_on(mem, mem->y);
            break;
        }

        case DEX: {
            cycles = 2;
            mem->x--;
            set_p_zn_on(mem, mem->x);
            break;
        }

        case INY: {
            mem->y++;
            cycles = 2;
            set_p_zn_on(mem, mem->y);
            break;
        }

        case INX: {
            mem->x++;
            cycles = 2;
            set_p_zn_on(mem, mem->x);
            break;
        }

        case INC_Absolute: {
            cycles = 6;
            uint16_t addr = read_address_and_inc_pc(mem);
            byte value = read_byte(mem, addr);
            value++;
            write_byte(mem, addr, value);
            set_p_zn_on(mem, value);
            break;
        }

        case INC_Zeropage: {
            cycles = 5;
            byte addr = read_byte_and_inc_pc(mem);
            byte value = read_byte(mem, addr);
            value++;
            write_byte(mem, addr, value);
            set_p_zn_on(mem, value);
            break;
        }

        case DEC_Absolute: {
            cycles = 3; // TODO Masswerk page says this is 3 cycles, but shouldn't it be 6 like INC_Absolute? Keeping it at 3 for now, verify later.
            uint16_t addr = read_address_and_inc_pc(mem);
            byte value = read_byte(mem, addr);
            value--;
            write_byte(mem, addr, value);
            set_p_zn_on(mem, value);
            break;
        }

        case DEC_Zeropage: {
            cycles = 5;
            byte addr = read_byte_and_inc_pc(mem);
            byte value = read_byte(mem, addr);
            value--;
            write_byte(mem, addr, value);
            set_p_zn_on(mem, value);
            break;
        }

        case DEC_Zeropage_X: {
            cycles = 6;
            byte addr = zeropage_x_address(mem);
            byte value = read_byte(mem, addr);
            value--;
            write_byte(mem, addr, value);
            set_p_zn_on(mem, value);
            break;
        }

        case BIT_Absolute: {
            cycles = 4;
            byte operand = read_value(mem, &cycles, Absolute);
            bool new_negative = (mask_flag(P_NEGATIVE) & operand) > 0;
            bool new_overflow = (mask_flag(P_OVERFLOW) & operand) > 0;
            set_p_negative_to(mem, new_negative);
            set_p_overflow_to(mem, new_overflow);
            set_p_zero_on(mem, operand & mem->a);
            break;
        }

        case ORA_Immediate: {
            cycles = 2;
            mem->a |= read_value(mem, &cycles, Immediate);
            set_p_zn_on(mem, mem->a);
            break;
        }

        case ORA_Zeropage: {
            cycles = 3;
            mem->a |= read_value(mem, &cycles, Zeropage);
            set_p_zn_on(mem, mem->a);
            break;
        }

        case AND_Immediate: {
            cycles = 2;
            mem->a &= read_value(mem, &cycles, Immediate);
            set_p_zn_on(mem, mem->a);
            break;
        }

        case AND_Zeropage: {
            cycles = 3;
            mem->a &= read_value(mem, &cycles, Zeropage);
            set_p_zn_on(mem, mem->a);
            break;
        }

        case AND_Zeropage_X: {
            cycles = 4;
            mem->a &= read_value(mem, &cycles, Zeropage_X);
            set_p_zn_on(mem, mem->a);
            break;
        }

        case AND_Absolute_X: {
            cycles = 4;
            mem->a &= read_value(mem, &cycles, Absolute_X);
            set_p_zn_on(mem, mem->a);
            break;
        }

        case JMP_Absolute: {
            cycles = 3;
            mem->pc = read_address_and_inc_pc(mem);
            break;
        }

        case JMP_Indirect: {
            cycles = 5;
            mem->pc = read_address_and_inc_pc(mem);
            // Jump to address read from first address
            mem->pc = read_address_and_inc_pc(mem);
            break;
        }

        case PHP: {
            php(mem);
            cycles = 3;
            break;
        }

        case PHA: {
            stack_push(mem, mem->a);
            cycles = 3;
            break;
        }

        case PLA: {
            mem->a = stack_pop(mem);
            set_p_zn_on(mem, mem->a);
            cycles = 4;
            break;
        }

        case LSR_Accumulator: {
            cycles = 2;
            set_p_carry_to(mem, mem->a & 1);
            mem->a >>= 1;
            set_p_zn_on(mem, mem->a);
            break;
        }

        case LSR_Zeropage: {
            cycles = 5;
            byte addr = read_byte_and_inc_pc(mem);
            byte value = read_byte(mem, addr);
            set_p_carry_to(mem, value & 1);
            value >>= 1;
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

        case ROR_Absolute_X: {
            cycles = 7;
            bool oldc = get_p_carry(mem);
            uint16_t address = absolute_x_address(mem, &cycles);
            byte value = read_byte(mem, address);
            set_p_carry_to(mem, value & 1);
            value = (value >> 1) | ((byte)oldc << 7);
            write_byte(mem, address, value);
            break;
        }

        case ROR_Zeropage: {
            cycles = 5;
            bool oldc = get_p_carry(mem);
            uint16_t address = read_byte_and_inc_pc(mem);
            byte value = read_byte(mem, address);
            set_p_carry_to(mem, value & 1);
            value = (value >> 1) | ((byte)oldc << 7);
            write_byte(mem, address, value);
            break;
        }

        case ROR_accumulator: {
            cycles = 2;
            bool oldc = get_p_carry(mem);
            set_p_carry_to(mem, mem->a & 1);
            mem->a = (mem->a >> 1) | ((byte)oldc << 7);
        }

        case SEC: {
            set_p_carry(mem);
            cycles = 2;
            break;
        }

        case SBC_Zeropage: {
            cycles = 3;
            sbc(mem, read_value(mem, &cycles, Zeropage));
            break;
        }

        case SBC_Absolute_Y: {
            cycles = 4;
            sbc(mem, read_value(mem, &cycles, Absolute_Y));
            break;
        }

        case EOR_Zeropage: {
            cycles = 3;
            mem->a ^= read_value(mem, &cycles, Zeropage);
            set_p_zn_on(mem, mem->a);
            break;
        }

        case EOR_Immediate: {
            cycles = 2;
            mem->a ^= read_value(mem, &cycles, Immediate);
            set_p_zn_on(mem, mem->a);
            break;
        }

        case CLC: {
            clear_p_carry(mem);
            cycles = 2;
            break;
        }

        case ASL_Accumulator: {
            cycles = 2;
            set_p_carry_to(mem, (mem->a >> 7) & 1);
            mem->a <<= 1;
            set_p_zn_on(mem, mem->a);
            break;
        }

        case ASL_Zeropage: {
            cycles = 5;
            byte addr = read_byte_and_inc_pc(mem);
            byte value = read_byte(mem, addr);
            set_p_carry_to(mem, (value >> 7) & 1);
            value <<= 1;
            set_p_zn_on(mem, value);
            write_byte(mem, addr, value);
            break;
        }

        case ADC_Zeropage: {
            cycles = 3;
            adc(mem, read_value(mem, &cycles, Zeropage));
            break;
        }

        case ADC_Immediate: {
            cycles = 2;
            adc(mem, read_value(mem, &cycles, Immediate));
            break;
        }

        case NOP: {
            cycles = 1;
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
    for (int i = 0; i < 0xFF; i++) {
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
