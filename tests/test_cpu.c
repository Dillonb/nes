#include "unity.h"
#include <src/cpu.h>
#include <src/opcode_names.h>
#include <src/mem.h>

memory mock_memory() {
    memory mem;

    mem.a = 0x00;
    mem.x = 0x00;
    mem.y = 0x00;
    mem.sp = 0xFD;
    mem.p = 0x34;
    mem.pc = 0x0000; // For tests, start reading at 0x0000 so we don't need to load a real ROM

    return mem;
}

void test_brk(void) {
    memory mem = mock_memory();
    write_byte(&mem, 0x0000, BRK);
    cpu_step(&mem);
    TEST_ASSERT_MESSAGE(get_p_interrupt(&mem) == 1, "Interrupt flag not set by BRK");
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_brk);
    //RUN_TEST(test_sei);
    return UNITY_END();
}
