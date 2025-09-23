#include "unity.h"
#include "c_string.h"

static c_string* make_string(const char* literal, int length) {
  return string_from_char(literal, length);
}

void setUp(void) {}

void tearDown(void) {}

void test_sub_string_returns_null_for_empty_source(void) {
  c_string* empty = make_string("", 0);
  c_string* slice = sub_string(empty, 0, 0);

  TEST_ASSERT_NULL_MESSAGE(slice, "sub_string should reject empty inputs instead of dereferencing NULL");

  destroy_string(empty);
}
