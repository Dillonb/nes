#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "system.h"
#include "cpu.h"
#include "apu.h"
#include "debugger.h"
#include "mem.h"
#include "mapper/rom.h"
#include "util.h"
#include "movie.h"
#include "render.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <rom.nes>\n", argv[0]);
        return 2;
    }

    if (argc > 2) {
        if (strcmp(argv[2], "debug") == 0) {
            set_debug();
            //set_breakpoint_on_interrupt();
            set_breakpoints_for_rom(argv[1]);
        }
    }


    rom* r = read_rom(argv[1]);

    memory* mem = get_blank_memory(r);

    if (argc > 3) {
        if (strcmp(argv[3], "interrupt") == 0) {
            set_breakpoint_on_interrupt();
        }
        else if (strcmp(argv[2], "movie") == 0) {
            set_movie_mode(read_movie(argv[3], mem));
        }
    }

    apu_init(&mem->apu_mem);

    while (true) {
        system_step(mem);
    }
}
