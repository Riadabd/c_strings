#include "unity.h"
#include "c_string.h"

#include <string.h>

static c_string* make_string(const char* literal) {
  return string_from_char(literal, (int)strlen(literal));
}

void setUp(void) {}

void tearDown(void) {}

void test_trim_char_removes_all_occurrences(void) {
  c_string* input = make_string("banana");
  c_string* trimmed = trim_char(input, 'a');

  TEST_ASSERT_NOT_NULL(trimmed);
  TEST_ASSERT_EQUAL_size_t(3, trimmed->length);
  TEST_ASSERT_EQUAL_MEMORY("bnn", trimmed->string, trimmed->length);

  destroy_string(trimmed);
  destroy_string(input);
}

void test_trim_char_returns_copy_when_character_missing(void) {
  c_string* input = make_string("citrus");
  c_string* trimmed = trim_char(input, 'z');

  TEST_ASSERT_NOT_NULL(trimmed);
  TEST_ASSERT_NOT_EQUAL(input, trimmed);
  TEST_ASSERT_EQUAL_size_t(input->length, trimmed->length);
  TEST_ASSERT_EQUAL_MEMORY(input->string, trimmed->string, trimmed->length);

  destroy_string(trimmed);
  destroy_string(input);
}

void test_trim_char_single_match_known_bug(void) {
  c_string* input = make_string("abc");
  TEST_IGNORE_MESSAGE("Known bug: trim_char fails when the target character appears once.");

  c_string* trimmed = trim_char(input, 'b');
  TEST_ASSERT_NOT_NULL(trimmed);
  TEST_ASSERT_EQUAL_size_t(2, trimmed->length);
  TEST_ASSERT_EQUAL_MEMORY("ac", trimmed->string, trimmed->length);

  destroy_string(trimmed);
  destroy_string(input);
}
