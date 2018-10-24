SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

add_executable (nes nes.c
    cpu.c
    cpu.h

    mem.c
    mem.h

    rom.c
    rom.h

    lib.c
    lib.h

    util.h
    )

add_executable (prgdump prgdump.c
    rom.c
    rom.h
    )

