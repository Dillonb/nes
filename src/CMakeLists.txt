SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

# For testing
add_library(libnes
    cpu.c
    mem.c
    rom.c
    ppu.c
    )


add_executable (nes nes.c
    cpu.c
    cpu.h

    mem.c
    mem.h

    rom.c
    rom.h

    lib.c
    lib.h

    system.c
    system.h

    ppu.c
    ppu.h

    util.h
    )

add_executable (prgdump prgdump.c
    rom.c
    rom.h
    )

