#include <err.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "rom.h"

int prg_offset_for_8kb_bank(rom* r, int bank) {
    if (bank < 0) {
        // Bank is negative, so this'll get the last bank for -1, 2nd to last for -2, etc
        bank = (r->header->prg_rom_blocks * 2) + bank; // prg_rom_blocks is given in 16kb units
    }

    if (bank >= 0x80) {
        bank -= 0x100;
    }
    bank %= (r->header->prg_rom_blocks * 2); // prg_rom_blocks is given in 16kb units
    int offset = bank * (0x2000);
    return offset;
}

int chr_offset_for_1kb_bank(rom *r, int bank) {
    int blocks = r->header->chr_rom_blocks;
    if (blocks == 0) {
        blocks = 1;
    }
    bank %= (blocks * 8); // chr_rom_blocks is given in 8kb units
    int offset = bank * 0x400;
    printf("CHR bank %d offset = 0x%04X\n", bank, offset);
    return offset;
}

void mapper4_init(rom* r) {
    // Switchable
    r->mapperdata.prg_bank_0_offset = prg_offset_for_8kb_bank(r, 0);
    r->mapperdata.prg_bank_1_offset = prg_offset_for_8kb_bank(r, 1);
    // Fixed
    r->mapperdata.prg_bank_2_offset = prg_offset_for_8kb_bank(r, -2);
    r->mapperdata.prg_bank_3_offset = prg_offset_for_8kb_bank(r, -1);

    r->mapperdata.prg_bank_mode = 0;
    r->mapperdata.chr_bank_mode = 0;

    r->mapperdata.ram_enabled = 1;
    r->mapperdata.ram_write_protect = 0;
    r->mapperdata.irq_enable = false;
}

byte mapper4_prg_read(rom* r, uint16_t address) {
    byte result;

    if (address < 0x6000) {
        errx(EXIT_FAILURE, "MMC3: Invalid PRG read at 0x%04X", address);
    }
    else if (address < 0x8000) {
        size_t prg_ram_bytes = get_prg_ram_bytes(r);
        address -= 0x6000;
        address %= prg_ram_bytes;
        result = r->prg_ram[address];
    }
    else if (address < 0xA000) {
        // 8KB switchable rom bank
        if (r->mapperdata.prg_bank_mode == 0) {
            result = r->prg_rom[r->mapperdata.prg_bank_0_offset + (address % 0x2000)];
        }
        else {
            result = r->prg_rom[r->mapperdata.prg_bank_2_offset + (address % 0x2000)];
        }
    }
    else if (address < 0xC000) {
        // 8KB switchable rom bank 2
        result = r->prg_rom[r->mapperdata.prg_bank_1_offset + (address % 0x2000)];
    }
    else if (address < 0xE000) {
        if (r->mapperdata.prg_bank_mode == 0) {
            result = r->prg_rom[r->mapperdata.prg_bank_2_offset + (address % 0x2000)];
        }
        else {
            result = r->prg_rom[r->mapperdata.prg_bank_0_offset + (address % 0x2000)];
        }
    }
    else { // <= 0xFFFF
        result = r->prg_rom[r->mapperdata.prg_bank_3_offset + (address % 0x2000)];
    }

    return result;
}

void mapper4_prg_write(rom* r, uint16_t address, byte value) {
    if (address < 0x6000) {
        errx(EXIT_FAILURE, "MMC3: Unhandled PRG write at 0x%04X", address);
    }
    else if (address < 0x8000) { // RAM
        size_t prg_ram_bytes = get_prg_ram_bytes(r);
        address -= 0x6000;
        address %= prg_ram_bytes;
        r->prg_ram[address] = value;
        printf("Wrote 0x%02X to 0x%04X\n", value, address + 0x6000);
    }
    else if (address < 0xA000 && address % 2 == 0) {
        // Bank select
        r->mapperdata.chr_bank_mode = (value & (byte)0b10000000) >> 7;
        r->mapperdata.prg_bank_mode = (value & (byte)0b01000000) >> 6;
        r->mapperdata.bank_register = value & (byte)0b111;
        printf("CHR bank mode: %d\nPRG bank mode: %d\nGonna update R%d next!\n",
               r->mapperdata.chr_bank_mode,
               r->mapperdata.prg_bank_mode,
               r->mapperdata.bank_register);
    }
    else if (address < 0xA000 && address % 2 == 1) {
        if (r->mapperdata.bank_register == 6 || r->mapperdata.bank_register == 7) {
            value &= (byte)0b00111111;
        }
        if (r->mapperdata.bank_register == 0 || r->mapperdata.bank_register == 1) {
            value &= (byte)0b11111110;
        }
        printf("Setting R%d to offset %d\n", r->mapperdata.bank_register, value);
        // Bank data
        switch (r->mapperdata.bank_register) {
            case 0:
                r->mapperdata.chr_bank_0_offset = chr_offset_for_1kb_bank(r, value);
                break;
            case 1:
                r->mapperdata.chr_bank_1_offset = chr_offset_for_1kb_bank(r, value);
                break;
            case 2:
                r->mapperdata.chr_bank_2_offset = chr_offset_for_1kb_bank(r, value);
                break;
            case 3:
                r->mapperdata.chr_bank_3_offset = chr_offset_for_1kb_bank(r, value);
                break;
            case 4:
                r->mapperdata.chr_bank_4_offset = chr_offset_for_1kb_bank(r, value);
                break;
            case 5:
                r->mapperdata.chr_bank_5_offset = chr_offset_for_1kb_bank(r, value);
                break;
            case 6:
                r->mapperdata.prg_bank_0_offset = prg_offset_for_8kb_bank(r, value);
                printf("R6 offset is now: 0x%04X\n", r->mapperdata.prg_bank_0_offset);
                break;
            case 7:
                r->mapperdata.prg_bank_1_offset = prg_offset_for_8kb_bank(r, value);
                printf("R7 offset is now: 0x%04X\n", r->mapperdata.prg_bank_1_offset);
                break;
            default:
                errx(EXIT_FAILURE, "MMC3: Bank data write: Unhandled bank number: %d", r->mapperdata.bank_register);
        }
    }
    else if (address < 0xC000 && address % 2 == 0) {
        // Mirroring
        if (value & (byte)1 == 0) {
            r->nametable_mirroring_mode = VERTICAL;
            printf("Nametable mirroring mode is now: VERTICAL\n");
        }
        else {
            r->nametable_mirroring_mode = HORIZONTAL;
            printf("Nametable mirroring mode is now: HORIZONTAL\n");
        }
    }
    else if (address < 0xC000 && address % 2 == 1) {
        // PRG RAM protect
        r->mapperdata.ram_enabled = (value >> 7) & (byte)1;
        r->mapperdata.ram_write_protect = (value >> 6) & (byte)1;
        printf("RAM enabled: %d\nRAM write protect: %d\n", r->mapperdata.ram_enabled, r->mapperdata.ram_write_protect);
    }
    else if (address < 0xE000 && address % 2 == 0) {
        // IRQ latch
        r->mapperdata.irq_latch = value;
    }
    else if (address < 0xE000 && address % 2 == 1) {
        // IRQ reload
        printf("TODO: implement this when I do IRQs\n");
    }
    else if (address <= 0xFFFF && address % 2 == 0) {
        // IRQ disable
        r->mapperdata.irq_enable = false;
        printf("IRQs are now DISABLED\n");
    }
    else if (address <= 0xFFFF && address % 2 == 1) {
        // IRQ enable
        r->mapperdata.irq_enable = true;
        printf("IRQs are now ENABLED\n");
    }
    else {
        errx(EXIT_FAILURE, "Tried to write 0x%02X to PRG at 0x%04X\n", value, address);
    }
}

int mapper4_get_chr_rom_index(rom *r, uint16_t address) {
    if (r->mapperdata.chr_bank_mode == 0) {
        if (address < 0x800) { // 2KB bank
            return r->mapperdata.chr_bank_0_offset + address;
        }
        else if (address < 0x1000) { // 2KB bank
            return r->mapperdata.chr_bank_1_offset + (address % 0x800);
        }
        else if (address < 0x1400) { // 1KB bank
            return r->mapperdata.chr_bank_2_offset + (address % 0x400);
        }
        else if (address < 0x1800) { // 1KB bank
            return r->mapperdata.chr_bank_3_offset + (address % 0x400);
        }
        else if (address < 0x1C00) { // 1KB bank
            return r->mapperdata.chr_bank_4_offset + (address % 0x400);
        }
        else if (address < 0x2000) { // 1KB bank
            return r->mapperdata.chr_bank_5_offset + (address % 0x400);
        }
        else {
            errx(EXIT_FAILURE, "Mapper 4: Unhandled CHR read at 0x%04X", address);
        }
    }
    else {
        if (address < 0x400) { // 1KB bank
            return r->mapperdata.chr_bank_2_offset + address;
        }
        else if (address < 0x800) { // 1KB bank
            return r->mapperdata.chr_bank_3_offset + (address % 0x400);
        }
        else if (address < 0xC00) { // 1KB bank
            return r->mapperdata.chr_bank_4_offset + (address % 0x400);
        }
        else if (address < 01000) { // 1KB bank
            return r->mapperdata.chr_bank_5_offset + (address % 0x400);
        }
        else if (address < 0x1800) { // 2KB bank
            return r->mapperdata.chr_bank_0_offset + (address % 0x800);
        }
        else if (address < 0x2000) { // 2KB bank
            return r->mapperdata.chr_bank_1_offset + (address % 0x800);
        }
        else {
            errx(EXIT_FAILURE, "Mapper 4: Unhandled CHR read at 0x%04X", address);
        }
    }
}

byte mapper4_chr_read(rom* r, uint16_t address) {
    return r->chr_rom[mapper4_get_chr_rom_index(r, address)];
}

void mapper4_chr_write(rom* r, uint16_t address, byte value) {
    errx(EXIT_FAILURE, "MMC3: Unhandled CHR write: 0x%04X = 0x%02X", address, value);
}
