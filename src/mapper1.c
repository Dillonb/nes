#include <err.h>
#include <stdlib.h>
#include "mapper1.h"

byte mapper1_prg_read(memory* mem, uint16_t address) {
    return 0x00;
}

byte prg_bank_mode = 0;
byte chr_bank_mode = 0;


byte chr_bank_0 = 0;
byte chr_bank_1 = 0;

byte ram_enabled;

byte prg_bank   = 0;

void load_register(byte shift_register, uint16_t address, memory* mem) {
    printf("Loading register (address 0x%04X) with value %02X\n", address, shift_register);
    if (address < 0x8000) {
        errx(EXIT_FAILURE, "Mapper 1: Tried to load register with an address less than 0x8000.");
    }
    else if (address < 0xA000) {
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
}

byte shift_register = 0x10;

void mapper1_prg_write(memory* mem, uint16_t address, byte value) {
    if (address < 0x6000) {
        errx(EXIT_FAILURE, "Wrote to invalid address for mapper 1: 0x%04X", address);
    }
    else if (address < 0x8000) {
        // 8kb prg ram bank
        errx(EXIT_FAILURE, "Need to implement PRG RAM bank for mapper 1: 0x%04X", address);
        // TODO make the prg_ram field on mem
        //mem->prg_ram[address - 0x6000] = value;
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