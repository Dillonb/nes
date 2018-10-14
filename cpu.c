#include <stdlib.h>
#include <err.h>

#include "cpu.h"
#include "mem.h"

#include "util.h"

char read_byte_and_inc_pc(memory* mem) {
    char byte = read_byte(mem, mem->pc);
    mem->pc++;
    return byte;
}

void cpu_step(memory* mem) {
    char opcode = read_byte_and_inc_pc(mem);

    switch (opcode) {
        default:
            errx(EXIT_FAILURE, "Invalid opcode: %x", opcode);
    }
}
