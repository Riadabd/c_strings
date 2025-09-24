#include "unity.h"
#include "c_string.h"

static c_string* make_string(const char* literal, int length) {
  CStringResult result = string_from_char(literal, length);
  TEST_ASSERT_EQUAL_INT(CSTRING_OK, result.status);
  TEST_ASSERT_NOT_NULL(result.value);
  return result.value;
}

void setUp(void) {}

void tearDown(void) {}

void test_sub_string_returns_null_for_empty_source(void) {
  c_string* empty = make_string("", 0);
  CStringResult slice = sub_string(empty, 0, 0);

  TEST_ASSERT_EQUAL_INT_MESSAGE(CSTRING_ERR_INVALID_ARG, slice.status,
                                "sub_string should reject empty inputs instead of dereferencing NULL");
  TEST_ASSERT_NULL(slice.value);

  destroy_string(empty);
}
