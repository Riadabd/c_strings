#include "unity.h"
#include "c_string.h"

#include <string.h>

static c_string* make_string(const char* literal) {
  CStringResult result = string_from_char(literal, (int)strlen(literal));
  TEST_ASSERT_EQUAL_INT(CSTRING_OK, result.status);
  TEST_ASSERT_NOT_NULL(result.value);
  return result.value;
}

void setUp(void) {}

void tearDown(void) {}

void test_trim_char_removes_all_occurrences(void) {
  c_string* input = make_string("banana");
  CStringResult trimmed = trim_char(input, 'a');

  TEST_ASSERT_EQUAL_INT(CSTRING_OK, trimmed.status);
  TEST_ASSERT_NOT_NULL(trimmed.value);
  TEST_ASSERT_EQUAL_size_t(3, trimmed.value->length);
  TEST_ASSERT_EQUAL_MEMORY("bnn", trimmed.value->string, trimmed.value->length);

  destroy_string(trimmed.value);
  destroy_string(input);
}

void test_trim_char_returns_copy_when_character_missing(void) {
  c_string* input = make_string("citrus");
  CStringResult trimmed = trim_char(input, 'z');

  TEST_ASSERT_EQUAL_INT(CSTRING_OK, trimmed.status);
  TEST_ASSERT_NOT_NULL(trimmed.value);
  TEST_ASSERT_NOT_EQUAL(input, trimmed.value);
  TEST_ASSERT_EQUAL_size_t(input->length, trimmed.value->length);
  TEST_ASSERT_EQUAL_MEMORY(input->string, trimmed.value->string, trimmed.value->length);

  destroy_string(trimmed.value);
  destroy_string(input);
}

void test_trim_char_single_match(void) {
  c_string* input = make_string("abc");

  CStringResult trimmed = trim_char(input, 'b');
  TEST_ASSERT_EQUAL_INT(CSTRING_OK, trimmed.status);
  TEST_ASSERT_NOT_NULL(trimmed.value);
  TEST_ASSERT_EQUAL_size_t(2, trimmed.value->length);
  TEST_ASSERT_EQUAL_MEMORY("ac", trimmed.value->string, trimmed.value->length);

  destroy_string(trimmed.value);
  destroy_string(input);
}

void test_trim_char_leading_match(void) {
  c_string* input = make_string("-abc");
  CStringResult trimmed = trim_char(input, '-');

  TEST_ASSERT_EQUAL_INT(CSTRING_OK, trimmed.status);
  TEST_ASSERT_NOT_NULL(trimmed.value);
  TEST_ASSERT_EQUAL_size_t(3, trimmed.value->length);
  TEST_ASSERT_EQUAL_MEMORY("abc", trimmed.value->string, trimmed.value->length);

  destroy_string(trimmed.value);
  destroy_string(input);
}

void test_trim_char_collapsed_runs(void) {
  c_string* input = make_string("a--b");
  CStringResult trimmed = trim_char(input, '-');

  TEST_ASSERT_EQUAL_INT(CSTRING_OK, trimmed.status);
  TEST_ASSERT_NOT_NULL(trimmed.value);
  TEST_ASSERT_EQUAL_size_t(2, trimmed.value->length);
  TEST_ASSERT_EQUAL_MEMORY("ab", trimmed.value->string, trimmed.value->length);

  destroy_string(trimmed.value);
  destroy_string(input);
}
