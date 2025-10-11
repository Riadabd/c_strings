#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "c_string.h"
#include "unity.h"

void setUp(void) {}

void tearDown(void) {}

static void expect_int_snprintf_match(int value) {
  char expected[32];
  int expected_len = snprintf(expected, sizeof(expected), "%d", value);
  TEST_ASSERT_TRUE(expected_len >= 0);
  TEST_ASSERT_LESS_THAN(
      (int)sizeof(expected),
      expected_len);  // TEST_ASSERT_LESS_THAN(threshold, actual) = Test if
                      // expected_len < (int)sizeof(expected)

  CStringResult result = int_to_string(value);
  TEST_ASSERT_EQUAL_INT(CSTRING_OK, result.status);
  TEST_ASSERT_NOT_NULL(result.value);
  TEST_ASSERT_EQUAL_UINT((unsigned int)expected_len, result.value->length);
  TEST_ASSERT_EQUAL_MEMORY(expected, result.value->string,
                           (size_t)expected_len);

  destroy_string(result.value);
}

static void expect_double_snprintf_match(double value) {
  char expected[64];
  int expected_len = snprintf(expected, sizeof(expected), "%g", value);
  TEST_ASSERT_TRUE(expected_len >= 0);
  TEST_ASSERT_LESS_THAN((int)sizeof(expected), expected_len);

  CStringResult result = double_to_string(value);
  TEST_ASSERT_EQUAL_INT(CSTRING_OK, result.status);
  TEST_ASSERT_NOT_NULL(result.value);
  TEST_ASSERT_EQUAL_UINT((unsigned int)expected_len, result.value->length);
  TEST_ASSERT_EQUAL_MEMORY(expected, result.value->string,
                           (size_t)expected_len);

  destroy_string(result.value);
}

void test_int_to_string_matches_snprintf_for_common_edges(void) {
  int values[] = {0, 1, -1, INT_MAX, INT_MIN};
  size_t size = sizeof(values) / sizeof(values[0]);
  for (size_t i = 0; i < size; i++) {
    expect_int_snprintf_match(values[i]);
  }
}

void test_double_to_string_matches_snprintf_for_common_cases(void) {
  double values[] = {0.0, -3.5, 1e-6, 1.2345e4, -9.87654e-3};
  size_t size = sizeof(values) / sizeof(values[0]);
  for (size_t i = 0; i < size; i++) {
    expect_double_snprintf_match(values[i]);
  }
}
