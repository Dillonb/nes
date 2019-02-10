SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

find_package(SDL2 REQUIRED)

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
    noop_render.c
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

    render.c
    render.h

    palette.h
    )

target_link_libraries(nes SDL2)


add_executable (prgdump prgdump.c
    rom.c
    rom.h
    )


add_executable (disassemble disassemble.c
    rom.c
    rom.h

    mem.c
    mem.h

    util.h

    cpu.c
    cpu.h

    ppu.c
    ppu.h

    opcode_names.c
    opcode_names.h

    debugger.c
    debugger.h

    set.c
    set.h

    util.c
    util.h

    noop_render.c
    render.h
    )
