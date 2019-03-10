SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

add_subdirectory(mapper)

find_package(SDL2 REQUIRED)
include_directories("/usr/local/include")
link_directories("/usr/local/lib")

# For testing
add_library(core
    util.c
    cpu.c
    mem.c
        mapper/rom.c
    ppu.c
    set.c
    debugger.c
    opcode_names.c
    noop_render.c
    system.c
    )

target_link_libraries(core mapper)


add_executable (nes nes.c
    cpu.c
    cpu.h

    mem.c
    mem.h

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

target_link_libraries(nes SDL2 mapper)


add_executable (prgdump prgdump.c
        mapper/rom.c
        mapper/rom.h
    )


add_executable (disassemble disassemble.c)
target_link_libraries(disassemble core)

