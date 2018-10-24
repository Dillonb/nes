add_subdirectory(unity)

add_executable(test_nes_cpu test_cpu.c)

target_link_libraries(test_nes_cpu unity)

add_test(test_nes_cpu test_nes_cpu)
