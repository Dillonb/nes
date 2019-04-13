#include <stdio.h>

#include "movie.h"
#include "cpu.h"

movie read_movie(char* filename, memory* mem) {
    printf("Loading a movie from %s\n", filename);
    movie m;
    m.handle = fopen(filename, "r");
    m.mem = mem;
    return m;
}

int offset = 1;

button_states get_next_inputs(movie* m) {
    button_states states;

    if (offset > 0) {
        states.states[0][0] = false;
        // B
        states.states[0][1] = false;
        // SELECT
        states.states[0][2] = false;
        // START
        states.states[0][3] = false;
        // UP
        states.states[0][4] = false;
        // DOWN
        states.states[0][5] = false;
        // LEFT
        states.states[0][6] = false;
        // RIGHT
        states.states[0][7] = false;

        offset--;
        return states;
    }
    ssize_t read;
    char* line = NULL;
    size_t len = 0;
    while ((read = getline(&line, &len, m->handle)) != -1 && line[0] != '|');

    printf("%s", line);
    // A
    states.states[0][0] = line[10] != '.' && line[10] != ' ';
    // B
    states.states[0][1] = line[9] != '.' && line[9] != ' ';
    // SELECT
    states.states[0][2] = line[8] != '.' && line[8] != ' ';
    // START
    states.states[0][3] = line[7] != '.' && line[7] != ' ';
    // UP
    states.states[0][4] = line[6] != '.' && line[6] != ' ';
    // DOWN
    states.states[0][5] = line[5] != '.' && line[5] != ' ';
    // LEFT
    states.states[0][6] = line[4] != '.' && line[4] != ' ';
    // RIGHT
    states.states[0][7] = line[3] != '.' && line[3] != ' ';

    // Soft reset
    if (line[1] == '1') {
        m->mem->pc = read_address(m->mem, 0xFFFC);
        m->mem->sp = 0xFD;
        m->mem->p  = 0x24;

        m->mem->ppu_mem.control     = 0b00000000;
        m->mem->ppu_mem.mask        = 0b00000000;
        m->mem->ppu_mem.status      = 0b10100000;
        m->mem->ppu_mem.oam_address = 0b00000000;
        m->mem->ppu_mem.v           = 0b0000000000000000;
        m->mem->ppu_mem.t           = 0b0000000000000000;
        m->mem->ppu_mem.w           = HIGH;
        m->mem->ppu_mem.data        = 0b00000000;

        m->mem->ppu_mem.tile.attribute_table  = 0;
        m->mem->ppu_mem.tile.nametable        = 0;
        m->mem->ppu_mem.tile.tile_bitmap_high = 0;
        m->mem->ppu_mem.tile.tile_bitmap_low  = 0;

        m->mem->ppu_mem.frame = 1;
        m->mem->ppu_mem.scan_line = 0;
        m->mem->ppu_mem.cycle = 0;

        m->mem->ppu_mem.num_sprites = 0;

        offset = 40;
        printf("RESETTING!\n");
    }

    return states;
}