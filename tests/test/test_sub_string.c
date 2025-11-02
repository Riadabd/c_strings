#include <string.h>

#include "c_string.h"
#include "unity.h"

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
  CStringResult slice = sub_string_checked(empty, 0, 0);

  TEST_ASSERT_EQUAL_INT_MESSAGE(
      CSTRING_ERR_INVALID_ARG, slice.status,
      "sub_string should reject empty inputs instead of dereferencing NULL");
  TEST_ASSERT_NULL(slice.value);

  destroy_string(empty);
}

void test_sub_string_codepoint_returns_multibyte_slice(void) {
  const char literal[] =
      "\xE2\x82\xAC\xE2\x98\x83\xE2\x9D\xA4";  // "€☃❤" in UTF-8
  c_string* source = make_string(literal, (int)strlen(literal));

  CStringResult slice = sub_string_codepoint(source, 1, 2);

  TEST_ASSERT_EQUAL_INT(CSTRING_OK, slice.status);
  TEST_ASSERT_NOT_NULL(slice.value);

  const char expected[] = "\xE2\x98\x83\xE2\x9D\xA4";  // "☃❤"
  TEST_ASSERT_EQUAL_size_t(strlen(expected), slice.value->length);
  char* actual = get_null_terminated_string(slice.value);
  TEST_ASSERT_EQUAL_STRING(expected, actual);
  TEST_ASSERT_TRUE(slice.value->utf8_valid);
  TEST_ASSERT_EQUAL_size_t(2, slice.value->codepoint_length);

  free(actual);
  destroy_string(slice.value);
  destroy_string(source);
}

void test_sub_string_codepoint_returns_single_multibyte_codepoint(void) {
  const char literal[] = "A"
                         "\xF0\x9F\x98\x80"
                         "B";  // "A😀B"
  c_string* source = make_string(literal, (int)strlen(literal));

  CStringResult slice = sub_string_codepoint(source, 1, 1);

  TEST_ASSERT_EQUAL_INT(CSTRING_OK, slice.status);
  TEST_ASSERT_NOT_NULL(slice.value);

  const char expected[] = "\xF0\x9F\x98\x80";  // "😀"
  TEST_ASSERT_EQUAL_size_t(strlen(expected), slice.value->length);
  char* actual = get_null_terminated_string(slice.value);
  TEST_ASSERT_EQUAL_STRING(expected, actual);
  TEST_ASSERT_TRUE(slice.value->utf8_valid);
  TEST_ASSERT_EQUAL_size_t(1, slice.value->codepoint_length);

  free(actual);
  destroy_string(slice.value);
  destroy_string(source);
}

void test_sub_string_codepoint_rejects_out_of_range_indices(void) {
  const char literal[] =
      "\xE2\x82\xAC\xE2\x98\x83\xE2\x9D\xA4";  // "€☃❤" in UTF-8
  c_string* source = make_string(literal, (int)strlen(literal));

  CStringResult slice = sub_string_codepoint(source, 0, 3);

  TEST_ASSERT_EQUAL_INT(CSTRING_ERR_INVALID_ARG, slice.status);
  TEST_ASSERT_NULL(slice.value);

  destroy_string(source);
}

void test_sub_string_codepoint_rejects_empty_source(void) {
  c_string* empty = make_string("", 0);

  CStringResult slice = sub_string_codepoint(empty, 0, 0);

  TEST_ASSERT_EQUAL_INT(CSTRING_ERR_INVALID_ARG, slice.status);
  TEST_ASSERT_NULL(slice.value);

  destroy_string(empty);
}

void test_sub_string_codepoint_rejects_start_after_end(void) {
  const char literal[] =
      "\xE2\x82\xAC\xE2\x98\x83\xE2\x9D\xA4";  // "€☃❤" in UTF-8
  c_string* source = make_string(literal, (int)strlen(literal));

  CStringResult slice = sub_string_codepoint(source, 2, 1);

  TEST_ASSERT_EQUAL_INT(CSTRING_ERR_INVALID_ARG, slice.status);
  TEST_ASSERT_NULL(slice.value);

  destroy_string(source);
}

void test_sub_string_codepoint_full_span_returns_original_bytes(void) {
  const char literal[] =
      "\xE2\x82\xAC\xE2\x98\x83\xE2\x9D\xA4";  // "€☃❤" in UTF-8
  c_string* source = make_string(literal, (int)strlen(literal));

  CStringResult slice = sub_string_codepoint(source, 0, 2);

  TEST_ASSERT_EQUAL_INT(CSTRING_OK, slice.status);
  TEST_ASSERT_NOT_NULL(slice.value);

  TEST_ASSERT_EQUAL_size_t(strlen(literal), slice.value->length);
  char* actual = get_null_terminated_string(slice.value);
  TEST_ASSERT_EQUAL_STRING(literal, actual);
  TEST_ASSERT_TRUE(slice.value->utf8_valid);
  TEST_ASSERT_EQUAL_size_t(source->codepoint_length, slice.value->codepoint_length);

  free(actual);
  destroy_string(slice.value);
  destroy_string(source);
}
