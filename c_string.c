#include "c_string.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ANSI Escape Codes for colours
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_WHITE "\x1B[37m"
#define ANSI_COLOR_RESET "\x1b[0m"

/* Helper Functions */

// Holds the result of validating a UTF-8 buffer so we can return both status
// and a code-point count without re-scanning.
// Boolean follows size_t to avoid clang warning about padding. Struct is still
// aligned at 16 bytes (8 + 1) , but this is now between the boolean and the
// struct boundary (instead of the boolean and size_t).
typedef struct {
  size_t codepoints;
  bool valid;
} Utf8Analysis;

static Utf8Analysis analyze_utf8(const char* data, size_t length) {
  Utf8Analysis result = {.valid = false, .codepoints = 0};

  if (!data) {
    if (length == 0) {
      result.valid = true;
    }
    return result;
  }

  size_t i = 0;
  size_t codepoints = 0;

  // Walk the buffer one UTF-8 sequence at a time until all bytes are consumed.
  while (i < length) {
    unsigned char byte = (unsigned char)data[i];
    size_t sequence_length = 0;
    uint32_t codepoint = 0;

    // Determine the expected length of the UTF-8 sequence from the lead byte.
    if (byte < 0x80) {
      // 0xxxxxxx → ASCII, single byte sequence.
      sequence_length = 1;
      codepoint = byte;
    } else if ((byte & 0xE0) == 0xC0) {
      // 110xxxxx → two-byte sequence.
      sequence_length = 2;
      codepoint = byte & 0x1F;
    } else if ((byte & 0xF0) == 0xE0) {
      // 1110xxxx → three-byte sequence.
      sequence_length = 3;
      codepoint = byte & 0x0F;
    } else if ((byte & 0xF8) == 0xF0) {
      // 11110xxx → four-byte sequence.
      sequence_length = 4;
      codepoint = byte & 0x07;
    } else {
      // Lead byte does not match any UTF-8 pattern.
      return result;
    }

    // Bail out if the buffer ends before the sequence completes.
    if (i + sequence_length > length) {
      return result;
    }

    for (size_t j = 1; j < sequence_length; j++) {
      unsigned char continuation = (unsigned char)data[i + j];
      // All continuation bytes must start with the "10" prefix.
      if ((continuation & 0xC0) != 0x80) {
        return result;
      }
      codepoint = (codepoint << 6) | (continuation & 0x3F);
    }

    // Reject overlong encodings and surrogate/invalid ranges to preserve UTF-8
    // invariants.
    if (sequence_length == 2 && codepoint < 0x80) {
      return result;  // overlong encoding
    }
    if (sequence_length == 3) {
      if (codepoint < 0x800) {
        return result;  // overlong encoding
      }
      if (codepoint >= 0xD800 && codepoint <= 0xDFFF) {
        return result;  // UTF-16 surrogate range is invalid in UTF-8
      }
    }
    if (sequence_length == 4) {
      if (codepoint < 0x10000 || codepoint > 0x10FFFF) {
        return result;
      }
    }

    // Successful sequence: advance to the next lead byte and bump the total.
    codepoints += 1;
    i += sequence_length;
  }

  result.valid = true;
  result.codepoints = codepoints;
  return result;
}

static bool update_utf8_metadata(c_string* s) {
  if (!s) {
    return false;
  }

  if (s->length == 0 || !s->string) {
    // Zero-length strings are trivially valid and contain zero code points.
    s->codepoint_length = 0;
    s->utf8_valid = (s->length == 0);
    return s->utf8_valid;
  }

  // Analyze once and cache the derived metadata on the string object.
  Utf8Analysis analysis = analyze_utf8(s->string, s->length);
  s->codepoint_length = analysis.codepoints;
  s->utf8_valid = analysis.valid;
  return analysis.valid;
}

// Create string from an input
void create_string(c_string* s, size_t length, char* input) {
  s->length = length;
  memcpy(s->string, input, s->length);
  if (!update_utf8_metadata(s)) {
    // Leave the string allocated but mark the metadata so callers can see the
    // parse failed.
    s->codepoint_length = 0;
    s->utf8_valid = false;
  }
}

// Initialize string buffer
CStringResult initialize_buffer(size_t length) {
  CStringResult result = {.value = NULL, .status = CSTRING_OK};

  c_string* data = calloc(1, sizeof(c_string));
  if (!data) {
    result.status = CSTRING_ERR_NO_MEMORY;
    return result;
  }

  data->length = length;

  if (length == 0) {
    data->string = NULL;
    data->codepoint_length = 0;
    data->utf8_valid = true;
    result.value = data;
    return result;
  }

  data->string = malloc(length);
  if (!data->string) {
    free(data);
    result.status = CSTRING_ERR_NO_MEMORY;
    return result;
  }

  // Caller must write valid UTF-8 into the buffer before the metadata can be
  // trusted.
  data->codepoint_length = 0;
  data->utf8_valid = false;
  result.value = data;
  return result;
}

// Copy contents of a c_string into a new one ("Copy Constructor")
CStringResult string_new(const c_string* s) {
  CStringResult result = {.value = NULL, .status = CSTRING_OK};

  if (!s) {
    result.status = CSTRING_ERR_INVALID_ARG;
    return result;
  }

  if (s->length > 0 && !s->string) {
    result.status = CSTRING_ERR_INVALID_ARG;
    return result;
  }

  c_string* new_s = calloc(1, sizeof(c_string));
  if (!new_s) {
    result.status = CSTRING_ERR_NO_MEMORY;
    return result;
  }

  new_s->length = s->length;

  if (s->length == 0) {
    new_s->string = NULL;
    new_s->codepoint_length = 0;
    new_s->utf8_valid = true;
    result.value = new_s;
    return result;
  }

  new_s->string = malloc(new_s->length);
  if (!new_s->string) {
    free(new_s);
    result.status = CSTRING_ERR_NO_MEMORY;
    return result;
  }

  memcpy(new_s->string, s->string, new_s->length);

  if (!update_utf8_metadata(new_s)) {
    // Copy succeeded at the byte level, but the contents are invalid UTF-8.
    free(new_s->string);
    free(new_s);
    result.status = CSTRING_ERR_INVALID_ARG;
    return result;
  }

  result.value = new_s;
  return result;
}

CStringResult string_from_char(const char* s, const int length) {
  CStringResult result = {.value = NULL, .status = CSTRING_OK};

  if (length < 0) {
    result.status = CSTRING_ERR_INVALID_ARG;
    return result;
  }

  c_string* new_s = calloc(1, sizeof(c_string));
  if (!new_s) {
    result.status = CSTRING_ERR_NO_MEMORY;
    return result;
  }

  if (length == 0) {
    new_s->string = NULL;
    new_s->length = 0;
    new_s->codepoint_length = 0;
    new_s->utf8_valid = true;
    result.value = new_s;
    return result;
  }

  if (!s) {
    // Non-zero length with NULL data is undefined; treat as invalid input.
    free(new_s);
    result.status = CSTRING_ERR_INVALID_ARG;
    return result;
  }

  new_s->length = (size_t)length;
  new_s->string = malloc(new_s->length);
  if (!new_s->string) {
    free(new_s);
    result.status = CSTRING_ERR_NO_MEMORY;
    return result;
  }
  memcpy(new_s->string, s, new_s->length);
  if (!update_utf8_metadata(new_s)) {
    // Reject malformed UTF-8 so we never hand back an invalid `c_string`.
    free(new_s->string);
    free(new_s);
    result.status = CSTRING_ERR_INVALID_ARG;
    return result;
  }

  result.value = new_s;
  return result;
}

// Start and end are inclusive bounds
CStringResult sub_string(c_string* s, size_t start, size_t end) {
  CStringResult result = {.value = NULL, .status = CSTRING_OK};

  if (!s || !s->string) {
    result.status = CSTRING_ERR_INVALID_ARG;
    return result;
  }

  if (s->length == 0) {
    result.status = CSTRING_ERR_INVALID_ARG;
    return result;
  }

  if (start > end || end >= s->length) {
    result.status = CSTRING_ERR_INVALID_ARG;
    return result;
  }

  size_t length = end - start + 1;
  c_string* new_s = calloc(1, sizeof(c_string));
  if (!new_s) {
    result.status = CSTRING_ERR_NO_MEMORY;
    return result;
  }

  new_s->length = length;

  if (length == 0) {
    new_s->string = NULL;
    result.value = new_s;
    return result;
  }

  new_s->string = malloc(length);
  if (!new_s->string) {
    free(new_s);
    result.status = CSTRING_ERR_NO_MEMORY;
    return result;
  }

  memcpy(new_s->string, s->string + start, length);

  result.value = new_s;
  return result;
}

// Return string inside c_string with a null-terminator in case an external
// function requires it
char* get_null_terminated_string(c_string* s) {
  char* result = malloc(s->length + 1);
  if (!result) {
    fputs("Memory allocation failure", stderr);
    exit(EXIT_FAILURE);
  }

  result[s->length] = '\0';
  memcpy(result, s->string, s->length);

  return result;
}

/* Free the string's content and the string itself */
void destroy_string(c_string* input) {
  free(input->string);
  free(input);
}

/* Help function used to free the memory used by the c_string** in string_delim
 */
void destroy_delim_string(c_string** s) {
  size_t i = 0;
  while (s[i] != NULL) {
    destroy_string(s[i]);
    i += 1;
  }

  free(s);
}

/* Concatenate input into string s */
void string_concat(c_string* s, const char* input) {
  // TODO: Check how to handle possible allocation size overflow
  s->string = realloc(s->string, s->length + strlen(input));
  if (!(s->string)) {
    fputs("Memory allocation failure", stderr);
    exit(EXIT_FAILURE);
  }

  memcpy(s->string + s->length, input, strlen(input));
  s->length += strlen(input);
}

void string_modify(c_string* s, const char* input) {
  size_t length = strlen(input);
  char* temp = realloc(s->string, length);
  if (temp != NULL) {
    s->string = temp;
    s->length = length;
    memcpy(s->string, input, length);
  }
}

/* Check if two strings are equal.
   Longer string is considered greater.
   Return > 0 if first is greater than second.
   Return 0 if they are equal.
   Return < 0 if second is greater than first. */
int string_compare(const c_string* first, const c_string* second) {
  if (first->length > second->length) {
    return 1;
  } else if (first->length < second->length) {
    return -1;
  }

  return memcmp(first->string, second->string, first->length);
}

/* Printing Functions */

void print(const c_string* s) { printf("%.*s", (int)s->length, s->string); }

void print_delim_strings(c_string** s) {
  size_t i = 0;
  printf("[");
  while (s[i + 1] != NULL) {
    printf("'");
    print(s[i]);
    printf("', ");
    i += 1;
  }
  printf("'");
  print(s[i]);
  printf("']");
}

// Supported Colors: Red, Green, Yellow, Blue, Magenta and Cyan
void print_colored(const c_string* s, const char* color) {
  if ((strcmp(color, "red") == 0) || (strcmp(color, "Red")) == 0) {
    printf(ANSI_COLOR_RED "%.*s" ANSI_COLOR_RESET, (int)s->length, s->string);
  } else if ((strcmp(color, "green") == 0) || (strcmp(color, "Green")) == 0) {
    printf(ANSI_COLOR_GREEN "%.*s" ANSI_COLOR_RESET, (int)s->length, s->string);
  } else if ((strcmp(color, "yellow") == 0) || (strcmp(color, "Yellow")) == 0) {
    printf(ANSI_COLOR_YELLOW "%.*s" ANSI_COLOR_RESET, (int)s->length,
           s->string);
  } else if ((strcmp(color, "blue") == 0) || (strcmp(color, "Blue")) == 0) {
    printf(ANSI_COLOR_BLUE "%.*s" ANSI_COLOR_RESET, (int)s->length, s->string);
  } else if ((strcmp(color, "magenta") == 0) ||
             (strcmp(color, "Magenta")) == 0) {
    printf(ANSI_COLOR_MAGENTA "%.*s" ANSI_COLOR_RESET, (int)s->length,
           s->string);
  } else if ((strcmp(color, "cyan") == 0) || (strcmp(color, "Cyan")) == 0) {
    printf(ANSI_COLOR_CYAN "%.*s" ANSI_COLOR_RESET, (int)s->length, s->string);
  } else if ((strcmp(color, "white") == 0) || (strcmp(color, "White")) == 0) {
    printf(ANSI_COLOR_WHITE "%.*s" ANSI_COLOR_RESET, (int)s->length, s->string);
  }
}

void print_utf8_info(const c_string* s) {
  if (!s) {
    printf("<null c_string>\n");
    return;
  }

  printf("string=%.*s bytes=%zu codepoints=%zu utf8_valid=%s\n", (int)s->length,
         s->string, s->length, s->codepoint_length,
         s->utf8_valid ? "true" : "false");
}

size_t get_delim_string_length(c_string** s) {
  size_t i = 0;
  while (s[i + 1] != NULL) {
    i += 1;
  }

  return i;
}

// Read stdin input into c_string
void scan_string(c_string* s, const char* prompt);

/* String Transformation Functions */

// Split string according to given delimiter.
//  * If delimiter size > input size, we consider that the delimiter was not
//  found and return the original input.
//  * If delimiter is exactly the input string, we return ['', ''] since we have
//  nothing "before" and "after" the delimiter match.
c_string** string_delim(const c_string* s, const char* delim) {
  // Count number of delimiter occurences.
  // We start the count at 1 since no match = returning the original string.
  size_t delim_counter = 1;
  const size_t delim_size = strlen(delim);

  if (delim_size > 0 && delim_size <= s->length) {
    for (size_t i = 0; i <= s->length - delim_size;) {
      if ((memcmp(s->string + i, delim, delim_size) == 0)) {
        delim_counter += 1;
        i += delim_size;
      } else {
        i += 1;
      }
    }
  }

  c_string** new_split_string = calloc(delim_counter + 1, sizeof(c_string*));
  if (!new_split_string) {
    return NULL;
  }

  // If we encounter no delimiter matches, return the original input string.
  // We allocate with size 2 to make use of a NULL terminator at the end.
  // This helps us print the resulting c_string*.
  if (delim_counter == 1) {
    CStringResult copy = string_new(s);
    if (copy.status != CSTRING_OK) {
      free(new_split_string);
      return NULL;
    }
    new_split_string[0] = copy.value;
    new_split_string[1] = NULL;
    return new_split_string;
  }

  size_t last_location = 0;
  size_t result_index = 0;
  for (size_t i = 0; i <= s->length - delim_size;) {
    if ((memcmp(s->string + i, delim, delim_size) == 0)) {
      if (i - last_location > 0) {
        CStringResult slice = string_from_char(s->string + last_location,
                                               (int)(i - last_location));
        if (slice.status != CSTRING_OK) {
          destroy_delim_string(new_split_string);
          return NULL;
        }
        new_split_string[result_index] = slice.value;
      } else {
        // There's nothing before the matched delimiter, so we add an empty
        // string.
        CStringResult empty = initialize_buffer(0);
        if (empty.status != CSTRING_OK) {
          destroy_delim_string(new_split_string);
          return NULL;
        }
        new_split_string[result_index] = empty.value;
      }

      last_location = i + delim_size;
      result_index += 1;
      i += delim_size;
    } else {
      i += 1;
    }
  }

  if (last_location < s->length) {
    CStringResult tail = string_from_char(s->string + last_location,
                                          (int)(s->length - last_location));
    if (tail.status != CSTRING_OK) {
      destroy_delim_string(new_split_string);
      return NULL;
    }
    new_split_string[result_index] = tail.value;
  } else {
    CStringResult empty = initialize_buffer(0);
    if (empty.status != CSTRING_OK) {
      destroy_delim_string(new_split_string);
      return NULL;
    }
    new_split_string[result_index] = empty.value;
  }
  new_split_string[result_index + 1] = NULL;

  return new_split_string;
}

CStringResult trim_char(const c_string* s, const char c) {
  CStringResult result = {.value = NULL, .status = CSTRING_OK};

  if (!s) {
    result.status = CSTRING_ERR_INVALID_ARG;
    return result;
  }

  if (s->length > 0 && !s->string) {
    result.status = CSTRING_ERR_INVALID_ARG;
    return result;
  }

  size_t num_of_occurences = 0;

  // Count number of character occurences
  for (size_t i = 0; i < s->length; i++) {
    if (s->string[i] == c) {
      num_of_occurences += 1;
    }
  }

  // If the input string does not contain the target character, return a copy of
  // the input string.
  if (num_of_occurences == 0) {
    return string_new(s);
  }

  // If the input string is the target character itself, return an empty buffer.
  if (s->length == num_of_occurences) {
    return initialize_buffer(0);
  }

  CStringResult buffer = initialize_buffer(s->length - num_of_occurences);
  if (buffer.status != CSTRING_OK) {
    return buffer;
  }

  c_string* result_string = buffer.value;

  size_t last_location = 0;
  size_t copy_position = 0;
  for (size_t i = 0; i < s->length; i++) {
    if (s->string[i] == c) {
      if (i > last_location) {
        memcpy(result_string->string + copy_position, s->string + last_location,
               i - last_location);
        copy_position += i - last_location;
      }

      // Increment last_location even if current index matches to avoid trailing
      // copies.
      last_location = i + 1;
    }
  }

  if (last_location < s->length) {
    memcpy(result_string->string + copy_position, s->string + last_location,
           s->length - last_location);
  }

  result.value = result_string;
  return result;
}

CStringResult to_lower(const c_string* s);

CStringResult string_from_printf(const char* fmt, ...) {
  CStringResult result = {.value = NULL, .status = CSTRING_OK};

  va_list args;
  va_start(args, fmt);
  int length = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  // Malformed format strings, encoding errors or other problems can make
  // vsnprintf return a negative value instead of a byte count.
  if (length < 0) {
    result.status = CSTRING_ERR_INTERNAL;
    return result;
  }

  size_t buf_size = (size_t)(length + 1);
  char* buf = malloc(buf_size);

  va_start(args, fmt);
  int written = vsnprintf(buf, buf_size, fmt, args);
  va_end(args);

  // Live state (locale changes, shared buffers) can diverge between the probe
  // and this write, so bail out if the second pass fails or needs more space.
  if (written < 0 || written > length) {
    free(buf);
    result.status = CSTRING_ERR_INTERNAL;
    return result;
  }

  CStringResult wrapped = string_from_char(buf, length);
  free(buf);
  return wrapped;
}

CStringResult int_to_string(int x) { return string_from_printf("%d", x); }

CStringResult double_to_string(double x) { return string_from_printf("%g", x); }

const char* cstring_status_str(CStringStatus status) {
  switch (status) {
    case CSTRING_OK:
      return "ok";
    case CSTRING_ERR_NO_MEMORY:
      return "no memory";
    case CSTRING_ERR_INVALID_ARG:
      return "invalid argument";
    case CSTRING_ERR_IO:
      return "i/o error";
    case CSTRING_ERR_OVERFLOW:
      return "overflow";
    case CSTRING_ERR_INTERNAL:
      return "internal error";
    default:
      return "unknown status";
  }
}
