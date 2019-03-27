#include <err.h>
#include <stdlib.h>
#include <stdint.h>

#include "rom.h"

void mapper31_init(mapper_data* mapperdata) {
    mapperdata->prg_bank_7_offset = 0xFF;
}

int mapper31_get_bank(uint16_t address) {
    return (address & 0x7000) >> 12; // _second_ three bits of address specify the bank (first is always 1)
}

int* mapper31_get_offset(int bank, mapper_data* mapperdata) {
    switch(bank) {
        case 0:
            return &mapperdata->prg_bank_0_offset;
        case 1:
            return &mapperdata->prg_bank_1_offset;
        case 2:
            return &mapperdata->prg_bank_2_offset;
        case 3:
            return &mapperdata->prg_bank_3_offset;
        case 4:
            return &mapperdata->prg_bank_4_offset;
        case 5:
            return &mapperdata->prg_bank_5_offset;
        case 6:
            return &mapperdata->prg_bank_6_offset;
        case 7:
            return &mapperdata->prg_bank_7_offset;
    }
}

byte mapper31_prg_read(rom* r, uint16_t address) {
    if (address >= 0x6000 && address < 0x8000) {
        size_t prg_ram_bytes = get_prg_ram_bytes(r);
        address -= 0x6000;
        address %= prg_ram_bytes;
        byte result = r->prg_ram[address];
        return result;
    }
    else if (address >= 0x8000) { // Can't be more than 0xFFFF
        int bank = mapper31_get_bank(address);
        int* offset = mapper31_get_offset(bank, &r->mapperdata);

        int prg_rom_address = (address % 0x1000) + (*offset * 0x1000);
        byte result = r->prg_rom[prg_rom_address];

        return result;
    }
    else {
        errx(EXIT_FAILURE, "Mapper 31: attempted to read at 0x%x, but this is not implemented (yet?)", address);
    }
}

void mapper31_prg_write(rom* r, uint16_t address, byte value) {
    if (address >= 0x6000 && address < 0x8000) {
        size_t prg_ram_bytes = get_prg_ram_bytes(r);
        address -= 0x6000;
        address %= prg_ram_bytes;
        r->prg_ram[address] = value;
    }
    else if (address >= 0x5000 && address <= 0x5fff){
        int bank_to_set = address & 0b111;
        int* offset = mapper31_get_offset(bank_to_set, &r->mapperdata);
        *offset = value;
    }
    else {
        printf("Tried to write 0x%02X to PRG at 0x%04X\n", value, address);
    }
}

byte mapper31_chr_read(rom* r, uint16_t address) {
    return r->chr_rom[address];
}

void mapper31_chr_write(rom* r, uint16_t address, byte value) {
    r->chr_rom[address] = value;
}
