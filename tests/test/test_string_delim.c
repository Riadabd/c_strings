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

  TEST_ASSERT_NOT_NULL_MESSAGE(pieces, "pieces array should not be NULL");
  TEST_ASSERT_NOT_NULL_MESSAGE(pieces[0], "first segment should not be NULL");
  TEST_ASSERT_NOT_NULL_MESSAGE(pieces[1], "second segment should not be NULL");
  TEST_ASSERT_NULL_MESSAGE(pieces[2], "third segment should be NULL (end of array)");

  TEST_ASSERT_EQUAL_size_t_MESSAGE(0, pieces[0]->length, "first segment should have length 0");
  TEST_ASSERT_NOT_NULL_MESSAGE(pieces[0]->string, "first segment string should not be NULL for empty segment");

  TEST_ASSERT_EQUAL_size_t_MESSAGE(3, pieces[1]->length, "second segment should have length 3");
  TEST_ASSERT_EQUAL_MEMORY_MESSAGE("ABC", pieces[1]->string, pieces[1]->length, "second segment should contain 'ABC'");

  destroy_delim_string(pieces);
  destroy_string(input);
}

void test_string_delim_empty_delimiter(void) {
  c_string* input = make_string("ABC");
  c_string** pieces = string_delim(input, "");

  TEST_ASSERT_NOT_NULL_MESSAGE(pieces, "pieces array should not be NULL");
  TEST_ASSERT_NOT_NULL_MESSAGE(pieces[0], "first segment should not be NULL");
  TEST_ASSERT_NULL_MESSAGE(pieces[1], "second segment should be NULL (end of array)");

  TEST_ASSERT_EQUAL_size_t_MESSAGE(3, pieces[0]->length, "first segment should have length 3");
  TEST_ASSERT_NOT_NULL_MESSAGE(pieces[0]->string, "first segment string should not be NULL");

  destroy_delim_string(pieces);
  destroy_string(input);
}

void test_string_delim_pointer_underflow(void) {
  c_string* input = make_string("ABC--DEF");
  c_string** pieces = string_delim(input, "--");

  TEST_ASSERT_NOT_NULL_MESSAGE(pieces, "pieces array should not be NULL");
  TEST_ASSERT_NOT_NULL_MESSAGE(pieces[0], "first segment should not be NULL");
  TEST_ASSERT_NOT_NULL_MESSAGE(pieces[1], "second segment should not be NULL");
  TEST_ASSERT_NULL_MESSAGE(pieces[2], "second segment should be NULL (end of array)");

  TEST_ASSERT_EQUAL_size_t_MESSAGE(3, pieces[0]->length, "first segment should have length 3");
  TEST_ASSERT_NOT_NULL_MESSAGE(pieces[0]->string, "first segment string should not be NULL");

  TEST_ASSERT_EQUAL_size_t_MESSAGE(3, pieces[1]->length, "second segment should have length 3");
  TEST_ASSERT_NOT_NULL_MESSAGE(pieces[1]->string, "second segment string should not be NULL");

  destroy_delim_string(pieces);
  destroy_string(input);
}

void test_string_delim_delimiter_longer_than_input(void) {
  c_string* input = make_string("abc");
  c_string** pieces = string_delim(input, "alphabet");

  TEST_ASSERT_NOT_NULL_MESSAGE(pieces, "pieces array should not be NULL");
  TEST_ASSERT_NOT_NULL_MESSAGE(pieces[0], "first segment should not be NULL");
  TEST_ASSERT_NULL_MESSAGE(pieces[1], "second segment should be NULL (end of array)");

  TEST_ASSERT_EQUAL_size_t_MESSAGE(input->length, pieces[0]->length, "original string should be preserved when delimiter is longer than input");
  TEST_ASSERT_EQUAL_MEMORY_MESSAGE(input->string, pieces[0]->string, pieces[0]->length, "segment should equal original string");

  destroy_delim_string(pieces);
  destroy_string(input);
}
