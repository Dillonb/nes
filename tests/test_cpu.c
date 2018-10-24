#include "unity.h"

void test_example(void) {
    TEST_ASSERT_EQUAL_INT(1,1);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_example);
    return UNITY_END();
}
