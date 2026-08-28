#include "unity.h"
#include "isotp.h"

void setUp(void) {}
void tearDown(void) {}

static void test_single_frame_is_recognised(void)
{
    TEST_ASSERT_EQUAL(ISOTP_PCI_SINGLE_FRAME, isotp_pci_type(0x07));
}

static void test_first_frame_is_recognised(void)
{
    TEST_ASSERT_EQUAL(ISOTP_PCI_FIRST_FRAME, isotp_pci_type(0x10));
}

static void test_reserved_nibble_is_invalid(void)
{
    TEST_ASSERT_EQUAL(ISOTP_PCI_INVALID, isotp_pci_type(0x40));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_single_frame_is_recognised);
    RUN_TEST(test_first_frame_is_recognised);
    RUN_TEST(test_reserved_nibble_is_invalid);
    return UNITY_END();
}
