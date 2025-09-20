#include "unity.h"
#include "c_string.h"

#include <string.h>

static c_string* make_string(const char* literal) {
  return string_from_char(literal, (int)strlen(literal));
}

void setUp(void) {}

void tearDown(void) {}

void test_string_delim_splits_multi_character_delimiter(void) {
  c_string* input = make_string("ABC--DEF--G");
  c_string** pieces = string_delim(input, "--");

  TEST_ASSERT_NOT_NULL(pieces);
  TEST_ASSERT_NOT_NULL(pieces[0]);
  TEST_ASSERT_NOT_NULL(pieces[1]);
  TEST_ASSERT_NOT_NULL(pieces[2]);
  TEST_ASSERT_NULL(pieces[3]);

  TEST_ASSERT_EQUAL_size_t(3, pieces[0]->length);
  TEST_ASSERT_EQUAL_MEMORY("ABC", pieces[0]->string, pieces[0]->length);
  TEST_ASSERT_EQUAL_size_t(3, pieces[1]->length);
  TEST_ASSERT_EQUAL_MEMORY("DEF", pieces[1]->string, pieces[1]->length);
  TEST_ASSERT_EQUAL_size_t(1, pieces[2]->length);
  TEST_ASSERT_EQUAL_MEMORY("G", pieces[2]->string, pieces[2]->length);

  destroy_delim_string(pieces);
  destroy_string(input);
}

void test_string_delim_contains_empty_segments_for_leading_delimiter(void) {
  c_string* input = make_string("--ABC");
  c_string** pieces = string_delim(input, "--");

  TEST_ASSERT_NOT_NULL(pieces);
  TEST_ASSERT_NOT_NULL(pieces[0]);
  TEST_ASSERT_NOT_NULL(pieces[1]);
  TEST_ASSERT_NULL(pieces[2]);

  TEST_ASSERT_EQUAL_size_t(0, pieces[0]->length);
  TEST_ASSERT_NULL(pieces[0]->string);

  TEST_ASSERT_EQUAL_size_t(3, pieces[1]->length);
  TEST_ASSERT_EQUAL_MEMORY("ABC", pieces[1]->string, pieces[1]->length);

  destroy_delim_string(pieces);
  destroy_string(input);
}
