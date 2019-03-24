#include <stdio.h>

#include "mapper/rom.h"
#include "mem.h"
#include "debugger.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("disassemble: disassemble PRG ROM of INES file\n");
        printf("Usage: %s <rom.nes>\n", argv[0]);
        return 2;
    }
    rom* r = read_rom(argv[1]);
    memory* mem = get_blank_memory(r);

    char* disassembly = disassemble(mem, mem->pc);

    printf("%s", disassembly);

    free(disassembly);
}
