#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "util.h"
#include "mem.h"
#include "ppu.h"

byte read_cartridge_space_address(rom* r, uint16_t address) {
    // http://wiki.nesdev.com/w/index.php/Mapper

    // NROM
    if (r->mapper == 0) {
        if (address >= 0x8000) { // Can't be more than 0xFFFF
            uint16_t prg_rom_address = (address - 0x8000) % get_prg_rom_bytes(r); // TODO optimize
            byte result = r->prg_rom[prg_rom_address];
            //printf("Reading cartridge space address 0x%x from PRG ROM at 0x%x: %02x\n", address, prg_rom_address, result);

            return result;
        }
    }
    errx(EXIT_FAILURE, "attempted to read_cartridge_space_address() at 0x%x, but this is not implemented (yet?)", address);
}

// http://wiki.nesdev.com/w/index.php/CPU_memory_map
byte read_byte(memory* mem, uint16_t address) {
    if (address < 0x2000) {
        return mem->ram[address % 0x800];
    }
    else if (address < 0x4000) { // PPU registers
        // 8 ppu registers, repeating every 8 bytes from 0x2000 to 0x3FFF
        byte register_num = (address - 2000) % 8;
        byte value = read_ppu_register(&mem->ppu_mem, register_num);
        printf("Read 0x%02x from PPU register %d\n", value, register_num);
        return value;
    }
    else if (address == 0x4016) {
        printf("Access to controller register #1 attempted, returning 0x00 for now.\n");
        return 0x00;
    }
    else if (address == 0x4017) {
        printf("Access to controller register #2 attempted, returning 0x00 for now.\n");
        return 0x00;
    }
    else if (address >= 0x4020) { // 0x4020 -> USHRT_MAX is cartridge space
        return read_cartridge_space_address(mem->r, address);
    }
    else {
        errx(EXIT_FAILURE, "Access attempted for invalid address: %x", address);
    }
}

void write_byte(memory* mem, uint16_t address, byte value) {
    if (address < 0x2000) { // RAM
        mem->ram[address % 0x800] = value;
    }
    else if (address < 0x4000) { // PPU registers
        // 8 ppu registers, repeating every 8 bytes from 0x2000 to 0x3FFF
        byte register_num = (address - 2000) % 8;
        printf("Writing 0x%02x to PPU register %d\n", value, register_num);
        write_ppu_register(&mem->ppu_mem, register_num, value);
    }
    else if (address == 0x4014) {
        errx(EXIT_FAILURE, "Write to OAM DMA register detected, but this is not implemented yet.");
    }
    else if (address < 0x4018) {
        printf("Write to APU register at address 0x%04x detected. Ignoring for now.\n", address);
    }
    else {
        errx(EXIT_FAILURE, "attempted to write_byte() 0x%02x at 0x%04x, but this is not implemented (yet?)", value, address);
    }
}

memory get_blank_memory(rom* r) {
    // http://wiki.nesdev.com/w/index.php/CPU_power_up_state
    memory mem;

    mem.a = 0x00;
    mem.x = 0x00;
    mem.y = 0x00;
    mem.sp = 0xFD;
    mem.p = 0x34;
    mem.r = r;
    mem.ppu_mem = get_ppu_mem();

    // Read initial value of program counter from the reset vector
    mem.pc = (read_cartridge_space_address(r, 0xFFFD) << 8) | read_cartridge_space_address(r, 0xFFFC);

    printf("Set program counter to 0x%x\n", mem.pc);

    return mem;
}

bool is_negative(byte value) {
    return (value & 0b10000000) > 0;
}

// 01-34567
// NV-BDIZC

void set_p_flag(memory* mem, int index) {
    mem->p |= mask_flag(index);
}
void clear_p_flag(memory* mem, int index) {
    mem->p &= ~mask_flag(index);
}
int get_p_flag(memory* mem, int index) {
    return (mem->p & mask_flag(index)) > 0;
}
void set_p_flag_to(memory* mem, int index, bool value) {
    if (value) {
        set_p_flag(mem, index);
    }
    else {
        clear_p_flag(mem, index);
    }
}

int get_p_negative(memory* mem) {
    return get_p_flag(mem, P_NEGATIVE);
}
int get_p_overflow(memory* mem) {
    return get_p_flag(mem, P_OVERFLOW);
}
int get_p_break(memory* mem) {
    return get_p_flag(mem, P_BREAK);
}
int get_p_decimal(memory* mem) {
    return get_p_flag(mem, P_DECIMAL);
}
int get_p_interrupt(memory* mem) {
    return get_p_flag(mem, P_INTERRUPT);
}
int get_p_zero(memory* mem) {
    return get_p_flag(mem, P_ZERO);
}
int get_p_carry(memory* mem) {
    return get_p_flag(mem, P_CARRY);
}

void set_p_negative(memory* mem) {
    set_p_flag(mem, P_NEGATIVE);
}
void set_p_overflow(memory* mem) {
    set_p_flag(mem, P_OVERFLOW);
}
void set_p_break(memory* mem) {
    set_p_flag(mem, P_BREAK);
}
void set_p_decimal(memory* mem) {
    set_p_flag(mem, P_DECIMAL);
}
void set_p_interrupt(memory* mem) {
    set_p_flag(mem, P_INTERRUPT);
}
void set_p_zero(memory* mem) {
    set_p_flag(mem, P_ZERO);
}
void set_p_carry(memory* mem) {
    set_p_flag(mem, P_CARRY);
}

void clear_p_negative(memory* mem) {
    clear_p_flag(mem, P_NEGATIVE);
}
void clear_p_overflow(memory* mem) {
    clear_p_flag(mem, P_OVERFLOW);
}
void clear_p_break(memory* mem) {
    clear_p_flag(mem, P_BREAK);
}
void clear_p_decimal(memory* mem) {
    clear_p_flag(mem, P_DECIMAL);
}
void clear_p_interrupt(memory* mem) {
    clear_p_flag(mem, P_INTERRUPT);
}
void clear_p_zero(memory* mem) {
    clear_p_flag(mem, P_ZERO);
}
void clear_p_carry(memory* mem) {
    clear_p_flag(mem, P_CARRY);
}

void set_p_negative_to(memory* mem, bool value) {
    set_p_flag_to(mem, P_NEGATIVE, value);
}
void set_p_overflow_to(memory* mem, bool value) {
    set_p_flag_to(mem, P_OVERFLOW, value);
}
void set_p_break_to(memory* mem, bool value) {
    set_p_flag_to(mem, P_BREAK, value);
}
void set_p_decimal_to(memory* mem, bool value) {
    set_p_flag_to(mem, P_DECIMAL, value);
}
void set_p_interrupt_to(memory* mem, bool value) {
    set_p_flag_to(mem, P_INTERRUPT, value);
}
void set_p_zero_to(memory* mem, bool value) {
    set_p_flag_to(mem, P_ZERO, value);
}
void set_p_carry_to(memory* mem, bool value) {
    set_p_flag_to(mem, P_CARRY, value);
}


void set_p_zero_on(memory* mem, byte value) {
    if (value == 0x00) {
        set_p_zero(mem);
    }
    else {
        clear_p_zero(mem);
    }
}

void set_p_negative_on(memory* mem, byte value) {
    if (is_negative(value)) {
        set_p_negative(mem);
    }
    else {
        clear_p_negative(mem);
    }
}

void set_p_zn_on(memory* mem, byte value) {
    set_p_zero_on(mem, value);
    set_p_negative_on(mem, value);
}

void stack_push(memory* mem, byte value) {
    write_byte(mem, 0x100 | mem->sp, value);
    mem->sp--;
}

void stack_push16(memory* mem, uint16_t value) {
    byte lower = value & 0xFF;
    byte higher = value >> 8;

    stack_push(mem, higher);
    stack_push(mem, lower);
}

byte stack_pop(memory* mem) {
    mem->sp++;
    return read_byte(mem, 0x100 | mem->sp);
}

uint16_t stack_pop16(memory* mem) {
    byte lower = stack_pop(mem);
    byte upper = stack_pop(mem);
    return (upper << 8) | lower;
}
