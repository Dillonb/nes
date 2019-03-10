SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

add_subdirectory(mapper)

find_package(SDL2 REQUIRED)
include_directories("/usr/local/include")
link_directories("/usr/local/lib")

add_library(core
        util.c
        util.h
        cpu.c
        cpu.h
        mem.c
        mem.h
        ppu.c
        ppu.h
        set.c
        set.h
        debugger.c
        debugger.h
        opcode_names.c
        opcode_names.h
        system.c
        system.h
        palette.h
        )

add_library(nooprender
        noop_render.c
        render.h
        )

add_library(render
        render.c
        render.h
        )

target_link_libraries(core mapper)


add_executable (nes nes.c)
target_link_libraries(nes SDL2 core render mapper)


add_executable (prgdump prgdump.c)
target_link_libraries(prgdump mapper)


add_executable (disassemble disassemble.c)
target_link_libraries(disassemble core nooprender)

