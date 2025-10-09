#include <string.h>

#include "c_string.h"
#include "unity.h"

static c_string* make_string(const char* literal) {
  CStringResult result = string_from_char(literal, (int)strlen(literal));
  TEST_ASSERT_EQUAL_INT(CSTRING_OK, result.status);
  TEST_ASSERT_NOT_NULL(result.value);
  return result.value;
}

void setUp(void) {}

void tearDown(void) {}

void test_string_from_char_counts_utf8_codepoints(void) {
  const char* literal = "héł🧊";  // 4 Unicode code points, 1 + 2 + 2 + 4 bytes
  CStringResult result = string_from_char(literal, (int)strlen(literal));

  TEST_ASSERT_EQUAL_INT(CSTRING_OK, result.status);
  TEST_ASSERT_NOT_NULL(result.value);
  TEST_ASSERT_EQUAL_size_t(strlen(literal), result.value->length);
  TEST_ASSERT_EQUAL_size_t(4, result.value->codepoint_length);
  TEST_ASSERT_TRUE(result.value->utf8_valid);

  destroy_string(result.value);
}

void test_string_from_char_rejects_invalid_utf8(void) {
  const char payload[] = {(char)0xC3, (char)0x28};  // invalid continuation
  CStringResult result = string_from_char(payload, (int)sizeof(payload));

  TEST_ASSERT_EQUAL_INT(CSTRING_ERR_INVALID_ARG, result.status);
  TEST_ASSERT_NULL(result.value);
}

void test_string_new_copies_utf8_metadata(void) {
  c_string* original = make_string("mañana");
  CStringResult copy_result = string_new(original);

  TEST_ASSERT_EQUAL_INT(CSTRING_OK, copy_result.status);
  TEST_ASSERT_NOT_NULL(copy_result.value);
  TEST_ASSERT_EQUAL_size_t(original->length, copy_result.value->length);
  TEST_ASSERT_EQUAL_size_t(original->codepoint_length,
                           copy_result.value->codepoint_length);
  TEST_ASSERT_EQUAL(original->utf8_valid, copy_result.value->utf8_valid);

  destroy_string(copy_result.value);
  destroy_string(original);
}

void test_create_string_populates_utf8_metadata(void) {
  const char* literal = "добро";  // 5 Cyrillic characters
  size_t bytes = strlen(literal);

  c_string* s = calloc(1, sizeof(c_string));
  TEST_ASSERT_NOT_NULL(s);
  s->string = malloc(bytes);
  TEST_ASSERT_NOT_NULL(s->string);

  create_string(s, bytes, (char*)literal);

  TEST_ASSERT_EQUAL_size_t(bytes, s->length);
  TEST_ASSERT_EQUAL_size_t(5, s->codepoint_length);
  TEST_ASSERT_TRUE(s->utf8_valid);

  destroy_string(s);
}

void test_sub_string_retains_utf8_metadata(void) {
  c_string* original = make_string("héł🧊");

  // Extract the first two code points (bytes 0-2 cover 'h' + two-byte 'é').
  CStringResult slice = sub_string(original, 0, 2);

  TEST_ASSERT_EQUAL_INT(CSTRING_OK, slice.status);
  TEST_ASSERT_NOT_NULL(slice.value);
  TEST_ASSERT_TRUE(slice.value->utf8_valid);
  TEST_ASSERT_EQUAL_size_t(2, slice.value->codepoint_length);

  destroy_string(slice.value);
  destroy_string(original);
}

void test_initialize_buffer_marks_unknown_utf8_until_filled(void) {
  CStringResult buffer = initialize_buffer(3);
  TEST_ASSERT_EQUAL_INT(CSTRING_OK, buffer.status);
  TEST_ASSERT_NOT_NULL(buffer.value);
  TEST_ASSERT_EQUAL_size_t(3, buffer.value->length);
  TEST_ASSERT_EQUAL_size_t(0, buffer.value->codepoint_length);
  TEST_ASSERT_FALSE(buffer.value->utf8_valid);

  destroy_string(buffer.value);
}

void test_initialize_buffer_zero_length_is_valid(void) {
  CStringResult buffer = initialize_buffer(0);
  TEST_ASSERT_EQUAL_INT(CSTRING_OK, buffer.status);
  TEST_ASSERT_NOT_NULL(buffer.value);
  TEST_ASSERT_EQUAL_size_t(0, buffer.value->length);
  TEST_ASSERT_EQUAL_size_t(0, buffer.value->codepoint_length);
  TEST_ASSERT_TRUE(buffer.value->utf8_valid);

  destroy_string(buffer.value);
}
