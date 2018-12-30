SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

# For testing
add_library(libnes
    util.c
    cpu.c
    mem.c
    rom.c
    ppu.c
    set.c
    debugger.c
    opcode_names.c
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

    set.c
    set.h

    util.c
    util.h

    debugger.c
    debugger.h

    opcode_names.c
    opcode_names.h
    )

add_executable (prgdump prgdump.c
    rom.c
    rom.h
    )

