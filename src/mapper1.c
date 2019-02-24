#include <err.h>
#include <stdlib.h>
#include "mapper1.h"


byte prg_bank_mode = 1;
byte chr_bank_mode = 0;

byte chr_bank_0 = 0;
byte chr_bank_1 = 0;

byte ram_enabled;

byte prg_bank = 0;

int prg_bank_0_offset = 0;
int prg_bank_1_offset = -1;

int chr_bank_0_offset = 0;
int chr_bank_1_offset = 0;

int prg_offset_for_bank(memory *mem, int bank) {
    if (bank >= 0x80) {
        bank -= 0x100;
    }
    bank %= mem->r->header->prg_rom_blocks;
    return bank * BYTES_PER_PRG_ROM_BLOCK;
}

int get_last_prg_bank(memory* mem) {
    return prg_offset_for_bank(mem, mem->r->header->prg_rom_blocks - 1);
}

int chr_offset_for_bank(memory *mem, int bank) {
    if (bank >= 0x80) {
        bank -= 0x100;
    }
    bank %= mem->r->header->chr_rom_blocks;
    return bank * BYTES_PER_CHR_ROM_BLOCK;
}

byte mapper1_prg_read(memory* mem, uint16_t address) {
    if (address < 0x6000) {
        errx(EXIT_FAILURE, "Mapper 1: Sub-0x6000 unsupported memory address read, 0x%04X", address);
    }
    else if (address < 0xC000) { // PRG bank 0, 0x8000 - 0xBFFF
        return mem->r->prg_rom[prg_bank_0_offset + (address  % 0x4000)];
    }
    else { // PRG bank 1, 0xC000 - 0xFFFF
        if (prg_bank_1_offset == -1) {
            prg_bank_1_offset = get_last_prg_bank(mem);
        }

        return mem->r->prg_rom[prg_bank_1_offset + (address % 0x4000)];
    }
}

void load_register(byte shift_register, uint16_t address, memory* mem) {
    printf("Loading register (address 0x%04X) with value %02X\n", address, shift_register);
    if (address < 0x8000) {
        errx(EXIT_FAILURE, "Mapper 1: Tried to load register with an address less than 0x8000.");
    }
    else if (address < 0xA000) { // write CONTROL register
        byte mirroring = shift_register & (byte)0b11;
        prg_bank_mode = (shift_register >> 2) & (byte)0b11;
        chr_bank_mode = (shift_register >> 4) & (byte)0b1;

        if (mirroring == 2) {
            mem->r->nametable_mirroring_mode = VERTICAL;
        }
        else if (mirroring == 3) {
            mem->r->nametable_mirroring_mode = HORIZONTAL;
        }
        else {
            errx(EXIT_FAILURE, "Mapper 1: unrecognized or unimplemented mirroring mode %d", mirroring);
        }
    }
    else if (address < 0xC000) {
        chr_bank_0 = shift_register;
    }
    else if (address < 0xE000) {
        chr_bank_1 = shift_register;
    }
    else {
        prg_bank = shift_register & (byte)0b1111;
        ram_enabled = (shift_register & (byte)0b10000) >> 4;
    }

    switch (prg_bank_mode) {
        case 0:
        case 1:
            prg_bank_0_offset = prg_offset_for_bank(mem, prg_bank & 0b1110);
            prg_bank_1_offset = prg_offset_for_bank(mem, prg_bank | 0b1);
            break;

        case 2:
            prg_bank_0_offset = prg_offset_for_bank(mem, 0);
            prg_bank_1_offset = prg_offset_for_bank(mem, prg_bank);
            break;

        case 3:
            prg_bank_0_offset = prg_offset_for_bank(mem, prg_bank);
            prg_bank_1_offset = get_last_prg_bank(mem);
            break;

        default:
            errx(EXIT_FAILURE, "Mapper 1: Unrecognized PRG bank mode %d", prg_bank_mode);
    }

    if (chr_bank_mode == 0) {
        chr_bank_0_offset = chr_offset_for_bank(mem, chr_bank_0 & 0b1110);
        chr_bank_1_offset = chr_offset_for_bank(mem, chr_bank_0 | 0b1);
    }
    else { // chr_bank_mode is only a single bit
        chr_bank_0_offset = chr_offset_for_bank(mem, chr_bank_0);
        chr_bank_1_offset = chr_offset_for_bank(mem, chr_bank_1);
    }
}

byte shift_register = 0x10;

void mapper1_prg_write(memory* mem, uint16_t address, byte value) {
    if (address < 0x6000) {
        errx(EXIT_FAILURE, "Wrote to invalid address for mapper 1: 0x%04X", address);
    }
    else if (address < 0x8000) {
        // 8kb prg ram bank
        mem->r->prg_ram[address - 0x6000] = value;
    }
    else if (address < 0xFFFF) {
        // TODO need to discard writes that happen on consecutive cycles, see https://wiki.nesdev.com/w/index.php/MMC1

        // Shift register stuff
        if (value > 0x7F) {
            // MSB not set, clear shift register
            shift_register = 0x10;
        }
        else {
            byte bit = (value & (byte)0b1) << 4;

            // as soon as the 1 from 0x10 has been shifted into the LSB, we're done.
            bool done_writing = (shift_register & 1) == 1;
            shift_register >>= 1;
            shift_register |= bit;

            if (done_writing) {
                load_register(shift_register, address, mem);
                shift_register = 0x10;
            }
        }
    }
}

byte mapper1_chr_read(ppu_memory* ppu_mem, uint16_t address) {
    int offset;
    if (address < 0x1000) {
        offset = chr_bank_0_offset;
    }
    else if (address < 0x2000) {
        offset = chr_bank_1_offset;
    }
    else {
        errx(EXIT_FAILURE, "Mapper 1: Attempt to read out of range CHR address 0x%04X", address);
    }
    return ppu_mem->r->chr_rom[offset + (address % 0x1000)];
}

void mapper1_chr_write(ppu_memory* ppu_mem, uint16_t address, byte value) {
}
