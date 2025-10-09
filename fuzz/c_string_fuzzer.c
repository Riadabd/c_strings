#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "c_string.h"

#ifndef __AFL_LOOP
#define __AFL_LOOP(_n) \
  for (int __afl_loop_dummy = 0; __afl_loop_dummy < 1; ++__afl_loop_dummy)
#endif

#ifndef __AFL_INIT
#define __AFL_INIT()
#endif

#define MAX_INPUT_SIZE (1 << 16)

static size_t read_input_file(const char* path, uint8_t* buffer,
                              size_t capacity) {
  if (!path) {
    return 0;
  }

  FILE* fp = fopen(path, "rb");
  if (!fp) {
    return 0;
  }

  size_t bytes_read = fread(buffer, 1, capacity, fp);
  fclose(fp);
  return bytes_read;
}

static void exercise_library(const uint8_t* data, size_t size) {
  if (!data || size > (size_t)INT_MAX) {
    return;
  }

  CStringResult base = string_from_char((const char*)data, (int)size);
  if (base.status != CSTRING_OK || !base.value) {
    return;
  }

  c_string* input = base.value;

  if (input->length > 0) {
    size_t start = 0;
    size_t end = 0;
    if (size > 0) {
      start = data[0] % input->length;
    }
    if (input->length > 1 && size > 1) {
      size_t span = data[1] % (input->length - start);
      end = start + span;
    } else {
      end = start;
    }

    CStringResult slice = sub_string(input, start, end);
    if (slice.status == CSTRING_OK && slice.value) {
      destroy_string(slice.value);
    }
  }

  if (size > 0) {
    char trim_char_candidate = (char)data[size - 1];
    CStringResult trimmed = trim_char(input, trim_char_candidate);
    if (trimmed.status == CSTRING_OK && trimmed.value) {
      destroy_string(trimmed.value);
    }
  }

  if (size > 0) {
    char delim_buf[3];
    delim_buf[0] = (char)data[0];
    delim_buf[1] = (size > 1) ? (char)data[size / 2] : '\0';
    delim_buf[2] = '\0';

    c_string** split = string_delim(input, delim_buf);
    if (split) {
      destroy_delim_string(split);
    }
  }

  destroy_string(input);
}

int main(int argc, char** argv) {
#ifdef __AFL_HAVE_MANUAL_CONTROL
  __AFL_INIT();
#endif

  static uint8_t buffer[MAX_INPUT_SIZE];
  const char* input_path = argc > 1 ? argv[1] : NULL;

  if (input_path) {
    while (__AFL_LOOP(1000)) {
      size_t size = read_input_file(input_path, buffer, sizeof(buffer));
      exercise_library(buffer, size);
    }
    return 0;
  }

  ssize_t len = read(STDIN_FILENO, buffer, sizeof(buffer));
  if (len > 0) {
    exercise_library(buffer, (size_t)len);
  }

  return 0;
}
