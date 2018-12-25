#include "unity/unity.h"
#include <src/mem.h>

memory mock_memory() {
    memory mem;

    mem.a = 0x00;
    mem.x = 0x00;
    mem.y = 0x00;
    mem.sp = 0xFD;
    mem.p = 0x34;
    mem.pc = 0x0000; // For tests, start reading at 0x0000 so we don't need to load a real ROM

    for (int i = 0; i < 0x800; i++) {
        mem.ram[i] = 0x00;
    }

    return mem;
}

void assert_value_at_address(memory* mem, uint16_t address, byte expected) {
    byte actual = read_byte(mem, address);
    TEST_ASSERT_EQUAL_UINT8(expected, actual);
}

void test_pflags() {
    memory mem = mock_memory();
    mem.p = 0x00;
    set_p_negative(&mem);
    TEST_ASSERT_EQUAL_UINT8(0b10000000, mem.p);
}

void test_stack() {
    memory mem_ = mock_memory();
    memory* mem = &mem_; // For convenience

    // Stack is empty
    assert_value_at_address(mem, 0x1FD, 0x00);

    // Push some values
    stack_push(mem, 0xDE);
    stack_push(mem, 0xAD);
    stack_push(mem, 0xBE);
    stack_push(mem, 0xEF);

    assert_value_at_address(mem, 0x1FD, 0xDE);
    assert_value_at_address(mem, 0x1FC, 0xAD);
    assert_value_at_address(mem, 0x1FB, 0xBE);
    assert_value_at_address(mem, 0x1FA, 0xEF);

    // Pop some values
    byte ef = stack_pop(mem);
    byte be = stack_pop(mem);
    byte ad = stack_pop(mem);
    byte de = stack_pop(mem);

    TEST_ASSERT_EQUAL_UINT8(de, 0xDE);
    TEST_ASSERT_EQUAL_UINT8(ad, 0xAD);
    TEST_ASSERT_EQUAL_UINT8(be, 0xBE);
    TEST_ASSERT_EQUAL_UINT8(ef, 0xEF);
}

void test_stack16() {
    memory mem_ = mock_memory();
    memory* mem = &mem_; // For convenience

    // Stack is empty
    assert_value_at_address(mem, 0x1FD, 0x00);

    stack_push16(mem, 0xCAFE);
    stack_push16(mem, 0xBABE);

    assert_value_at_address(mem, 0x1FD, 0xCA);
    assert_value_at_address(mem, 0x1FC, 0xFE);
    assert_value_at_address(mem, 0x1FB, 0xBA);
    assert_value_at_address(mem, 0x1FA, 0xBE);

    uint16_t babe = stack_pop16(mem);
    uint16_t cafe = stack_pop16(mem);

    TEST_ASSERT_EQUAL_UINT16(0xCAFE, cafe);
    TEST_ASSERT_EQUAL_UINT16(0xBABE, babe);

}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_pflags);
    RUN_TEST(test_stack);
    RUN_TEST(test_stack16);
    return UNITY_END();
}
