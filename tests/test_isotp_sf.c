#include "unity.h"
#include "isotp.h"

#include <string.h>

/* --- fixture: captures whatever isotp_on_frame delivered via on_indication --- */

static uint8_t  captured_data[8U];
static uint16_t captured_len;
static int      captured_call_count;

static void capture_indication(const uint8_t *data, uint16_t len, void *user_data)
{
    (void)user_data;
    captured_call_count++;
    captured_len = len;
    memcpy(captured_data, data, (len < sizeof(captured_data)) ? len : sizeof(captured_data));
}

static isotp_ctx_t ctx;
static uint8_t     rx_buf[8];

void setUp(void)
{
    captured_call_count = 0;
    captured_len = 0U;
    memset(captured_data, 0U, sizeof(captured_data));
    isotp_init(&ctx, rx_buf, sizeof(rx_buf), capture_indication, NULL);
}

void tearDown(void) {}

/* ---- valid SF ---- */

static void test_sf_minimal_one_byte_payload(void)
{
    /* SF_DL=1, one payload byte, frame len=2
       -> ISOTP_RX_OK, callback fires once with len=1 and the correct byte */
    uint8_t const frame[]    = {0x01, 0xAA};
    uint8_t const expected[] = {0xAA};

    isotp_rx_status_t const status =
        isotp_on_frame(&ctx, frame, (uint8_t)sizeof(frame), 0U);

    TEST_ASSERT_EQUAL(ISOTP_RX_OK, status);
    TEST_ASSERT_EQUAL_INT(1, captured_call_count);
    TEST_ASSERT_EQUAL_UINT16(1U, captured_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, captured_data, sizeof(expected));
}

static void test_sf_typical_three_byte_payload(void)
{
    /* SF_DL=3, exact frame len=4, no padding
       -> ISOTP_RX_OK, callback data matches the 3 payload bytes exactly */
    uint8_t const frame[]    = {0x03, 0xAA, 0xBB, 0xCC};
    uint8_t const expected[] = {0xAA, 0xBB, 0xCC};

    isotp_rx_status_t const status =
        isotp_on_frame(&ctx, frame, (uint8_t)sizeof(frame), 0U);

    TEST_ASSERT_EQUAL(ISOTP_RX_OK, status);
    TEST_ASSERT_EQUAL_INT(1, captured_call_count);
    TEST_ASSERT_EQUAL_UINT16(3U, captured_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, captured_data, sizeof(expected));
}

static void test_sf_maximum_seven_byte_payload(void)
{
    /* SF_DL=7, frame len=8 (max payload a classic CAN SF can carry)
       -> ISOTP_RX_OK */
    uint8_t const frame[]    = {0x07, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11};
    uint8_t const expected[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11};

    isotp_rx_status_t const status =
        isotp_on_frame(&ctx, frame, (uint8_t)sizeof(frame), 0U);

    TEST_ASSERT_EQUAL(ISOTP_RX_OK, status);
    TEST_ASSERT_EQUAL_INT(1, captured_call_count);
    TEST_ASSERT_EQUAL_UINT16(7U, captured_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, captured_data, sizeof(expected));
}

/* ---- malformed SF ---- */

static void test_sf_zero_length_is_malformed(void)
{
    /* SF_DL=0 -> ISOTP_RX_MALFORMED, callback must not fire */
    uint8_t const frame[]    = {0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11};

    isotp_rx_status_t const status =
        isotp_on_frame(&ctx, frame, (uint8_t)sizeof(frame), 0U);

    TEST_ASSERT_EQUAL(ISOTP_RX_MALFORMED, status);
    TEST_ASSERT_EQUAL_INT(0, captured_call_count);
    TEST_ASSERT_EQUAL_UINT16(0U, captured_len);
}

static void test_sf_truncated_is_malformed(void)
{
    /* SF_DL=5 but frame only carries 3 bytes total -> ISOTP_RX_MALFORMED */
    uint8_t const frame[]    = {0x05, 0xAA, 0xBB, 0xCC, 0xDD};

    isotp_rx_status_t const status =
        isotp_on_frame(&ctx, frame, (uint8_t)sizeof(frame), 0U);

    TEST_ASSERT_EQUAL(ISOTP_RX_MALFORMED, status);
    TEST_ASSERT_EQUAL_INT(0, captured_call_count);
    TEST_ASSERT_EQUAL_UINT16(0U, captured_len);
}

static void test_sf_empty_frame_is_malformed(void)
{
    /* len=0: nothing to even read a PCI byte from, must not touch data[0].
       Confirm expected status against isotp_on_frame's current behaviour
       for len==0 before asserting. */
    uint8_t const frame[]    = {0x00};

    isotp_rx_status_t const status =
        isotp_on_frame(&ctx, frame, (uint8_t)sizeof(frame), 0U);

    TEST_ASSERT_EQUAL(ISOTP_RX_MALFORMED, status);
    TEST_ASSERT_EQUAL_INT(0, captured_call_count);
    TEST_ASSERT_EQUAL_UINT16(0U, captured_len);
}

static void test_sf_pci_byte_only_is_malformed(void)
{
    /* len=1: PCI byte present but zero payload bytes -> ISOTP_RX_MALFORMED */
    uint8_t const frame[]    = {0x01};

    isotp_rx_status_t const status =
        isotp_on_frame(&ctx, frame, (uint8_t)sizeof(frame), 0U);

    TEST_ASSERT_EQUAL(ISOTP_RX_MALFORMED, status);
    TEST_ASSERT_EQUAL_INT(0, captured_call_count);
    TEST_ASSERT_EQUAL_UINT16(0U, captured_len);
}

static void test_sf_padded_frame_status(void)
{
    /* SF_DL=3 but frame padded out to len=8. isotp_on_single_frame requires
       exact len == SF_DL+1, so padding is rejected as malformed. */
    uint8_t const frame[] = {0x03U, 0xAAU, 0xBBU, 0xCCU, 0xCCU, 0xCCU, 0xCCU, 0xCCU};

    isotp_rx_status_t const status =
        isotp_on_frame(&ctx, frame, (uint8_t)sizeof(frame), 0U);

    TEST_ASSERT_EQUAL(ISOTP_RX_MALFORMED, status);
    TEST_ASSERT_EQUAL_INT(0, captured_call_count);
    TEST_ASSERT_EQUAL_UINT16(0U, captured_len);
}

/* ---- non-SF frame types: not handled yet ---- */

static void test_first_frame_is_ignored(void)
{
    /* nibble 0x1 -> ISOTP_RX_IGNORED, callback must not fire.
       FF_DL = 0x023 = 35 bytes; nothing parses it yet. */
    uint8_t const frame[] = {0x10U, 0x23U, 0xAAU, 0xBBU, 0xCCU, 0xDDU, 0xEEU, 0xFFU};

    isotp_rx_status_t const status =
        isotp_on_frame(&ctx, frame, (uint8_t)sizeof(frame), 0U);

    TEST_ASSERT_EQUAL(ISOTP_RX_IGNORED, status);
    TEST_ASSERT_EQUAL_INT(0, captured_call_count);
    TEST_ASSERT_EQUAL_UINT16(0U, captured_len);
}

static void test_consecutive_frame_is_ignored(void)
{
    /* nibble 0x2 -> ISOTP_RX_IGNORED */
    uint8_t const frame[] = {0x20U, 0x23U, 0xAAU, 0xBBU, 0xCCU, 0xDDU, 0xEEU, 0xFFU};

    isotp_rx_status_t const status =
        isotp_on_frame(&ctx, frame, (uint8_t)sizeof(frame), 0U);

    TEST_ASSERT_EQUAL(ISOTP_RX_IGNORED, status);
    TEST_ASSERT_EQUAL_INT(0, captured_call_count);
    TEST_ASSERT_EQUAL_UINT16(0U, captured_len);
}

static void test_flow_control_is_ignored(void)
{
    /* nibble 0x3 -> ISOTP_RX_IGNORED */
    uint8_t const frame[] = {0x30U, 0x23U, 0xAAU, 0xBBU, 0xCCU, 0xDDU, 0xEEU, 0xFFU};

    isotp_rx_status_t const status =
        isotp_on_frame(&ctx, frame, (uint8_t)sizeof(frame), 0U);

    TEST_ASSERT_EQUAL(ISOTP_RX_IGNORED, status);
    TEST_ASSERT_EQUAL_INT(0, captured_call_count);
    TEST_ASSERT_EQUAL_UINT16(0U, captured_len);
}

static void test_reserved_nibble_status(void)
{
    /* nibble 0x4-0xF: ISOTP_RX_MALFORMED */
    uint8_t const frame[] = {0x40U, 0x23U, 0xAAU, 0xBBU, 0xCCU, 0xDDU, 0xEEU, 0xFFU};

    isotp_rx_status_t const status =
        isotp_on_frame(&ctx, frame, (uint8_t)sizeof(frame), 0U);

    TEST_ASSERT_EQUAL(ISOTP_RX_MALFORMED, status);
    TEST_ASSERT_EQUAL_INT(0, captured_call_count);
    TEST_ASSERT_EQUAL_UINT16(0U, captured_len);
}

/* ---- robustness ---- */

static void test_two_sequential_sf_do_not_bleed_state(void)
{
    /* two separate valid SFs via two isotp_on_frame calls -> callback fires
       twice, once per frame, each with its own correct payload.
       Long then short on purpose: rx_buf still holds the tail of the first
       message, so the second delivery must report len=2 and not leak it. */
    uint8_t const first[]  = {0x07U, 0x11U, 0x22U, 0x33U, 0x44U, 0x55U, 0x66U, 0x77U};
    uint8_t const second[] = {0x02U, 0xAAU, 0xBBU};

    uint8_t const expected_first[]  = {0x11U, 0x22U, 0x33U, 0x44U, 0x55U, 0x66U, 0x77U};
    uint8_t const expected_second[] = {0xAAU, 0xBBU};

    isotp_rx_status_t status =
        isotp_on_frame(&ctx, first, (uint8_t)sizeof(first), 0U);

    TEST_ASSERT_EQUAL(ISOTP_RX_OK, status);
    TEST_ASSERT_EQUAL_INT(1, captured_call_count);
    TEST_ASSERT_EQUAL_UINT16(7U, captured_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_first, captured_data, sizeof(expected_first));

    status = isotp_on_frame(&ctx, second, (uint8_t)sizeof(second), 0U);

    TEST_ASSERT_EQUAL(ISOTP_RX_OK, status);
    TEST_ASSERT_EQUAL_INT(2, captured_call_count);
    TEST_ASSERT_EQUAL_UINT16(2U, captured_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_second, captured_data, sizeof(expected_second));
}

static void test_small_rx_buf_rejects_sf_that_would_overflow_it(void)
{
    /* Re-init with a buffer too small for the incoming SF: the frame itself is
       well formed, so this is ISOTP_RX_OVERFLOW rather than ISOTP_RX_MALFORMED,
       and nothing may be copied or delivered. */
    uint8_t small_buf[3] = {0U, 0U, 0U};
    uint8_t const frame[] = {0x05U, 0x11U, 0x22U, 0x33U, 0x44U, 0x55U};

    isotp_init(&ctx, small_buf, (uint16_t)sizeof(small_buf), capture_indication, NULL);

    isotp_rx_status_t const status =
        isotp_on_frame(&ctx, frame, (uint8_t)sizeof(frame), 0U);

    TEST_ASSERT_EQUAL(ISOTP_RX_OVERFLOW, status);
    TEST_ASSERT_EQUAL_INT(0, captured_call_count);
    TEST_ASSERT_EQUAL_UINT16(0U, captured_len);

    /* the too-small buffer must be left untouched */
    TEST_ASSERT_EACH_EQUAL_UINT8(0U, small_buf, sizeof(small_buf));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_sf_minimal_one_byte_payload);
    RUN_TEST(test_sf_typical_three_byte_payload);
    RUN_TEST(test_sf_maximum_seven_byte_payload);
    RUN_TEST(test_sf_zero_length_is_malformed);
    RUN_TEST(test_sf_truncated_is_malformed);
    RUN_TEST(test_sf_empty_frame_is_malformed);
    RUN_TEST(test_sf_pci_byte_only_is_malformed);
    RUN_TEST(test_sf_padded_frame_status);
    RUN_TEST(test_first_frame_is_ignored);
    RUN_TEST(test_consecutive_frame_is_ignored);
    RUN_TEST(test_flow_control_is_ignored);
    RUN_TEST(test_reserved_nibble_status);
    RUN_TEST(test_two_sequential_sf_do_not_bleed_state);
    RUN_TEST(test_small_rx_buf_rejects_sf_that_would_overflow_it);
    return UNITY_END();
}
