add_subdirectory(unity)

add_executable(test_nes_cpu test_cpu.c)
add_executable(test_nes_mem test_mem.c)

target_link_libraries(test_nes_cpu libnes unity)
target_link_libraries(test_nes_mem libnes unity)

add_test(test_nes_cpu test_nes_cpu)
add_test(test_nes_mem test_nes_mem)

target_include_directories(test_nes_cpu PUBLIC .. src)
target_include_directories(test_nes_mem PUBLIC .. src)
