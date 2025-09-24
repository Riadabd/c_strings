#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_string.h"

// ANSI Escape Codes for colours
#define ANSI_COLOR_RED      "\x1b[31m"
#define ANSI_COLOR_GREEN    "\x1b[32m"
#define ANSI_COLOR_YELLOW   "\x1b[33m"
#define ANSI_COLOR_BLUE     "\x1b[34m"
#define ANSI_COLOR_MAGENTA  "\x1b[35m"
#define ANSI_COLOR_CYAN     "\x1b[36m"
#define ANSI_COLOR_WHITE    "\x1B[37m"
#define ANSI_COLOR_RESET    "\x1b[0m"

/* Helper Functions */

// Convert integer value to char*
static char* itoa_c(int value, char* result, int base) {
  // Check if the base is valid
  if (base < 2 || base > 36) {
    result = NULL;
    return result;
  }

  char* ptr = result, *ptr1 = result, tmp_char;
  int tmp_value;

  do {
    tmp_value = value;
    value /= base;
    *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
  } while (value);

  // Apply negative sign
  if (tmp_value < 0) *ptr++ = '-';
  //*ptr-- = '\0';
  ptr--;
  while (ptr1 < ptr) {
    tmp_char = *ptr;
    *ptr--= *ptr1;
    *ptr1++ = tmp_char;
  }

  return result;
}

// Count number of digits
static unsigned int count_bchop(unsigned int n) {
  int r = 1;

  if (n >= 100000000) {
    r += 8;
    n /= 100000000;
  }

  if (n >= 10000) {
    r += 4;
    n /= 10000;
  }

  if (n >= 100) {
    r += 2;
    n /= 100;
  }

  if (n >= 10) {
    r += 1;
  }

  return r;
}

// Create string from an input
void create_string(c_string* s, size_t length, char* input) {
  s->length = length;
  memcpy(s->string, input, s->length);
}

// Initialize string buffer
CStringResult initialize_buffer(size_t length) {
  CStringResult result = { .value = NULL, .status = CSTRING_OK };

  c_string* data = calloc(1, sizeof(c_string));
  if (!data) {
    result.status = CSTRING_ERR_NO_MEMORY;
    return result;
  }

  data->length = length;

  if (length == 0) {
    data->string = NULL;
    result.value = data;
    return result;
  }

  data->string = malloc(length);
  if (!data->string) {
    free(data);
    result.status = CSTRING_ERR_NO_MEMORY;
    return result;
  }

  result.value = data;
  return result;
}

// Copy contents of a c_string into a new one ("Copy Constructor")
CStringResult string_new(const c_string* s) {
  CStringResult result = { .value = NULL, .status = CSTRING_OK };

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

  result.value = new_s;
  return result;
}

CStringResult string_from_char(const char* s, const int length) {
  CStringResult result = { .value = NULL, .status = CSTRING_OK };

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
    result.value = new_s;
    return result;
  }

  if (!s) {
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

  result.value = new_s;
  return result;
}

// Start and end are inclusive bounds
CStringResult sub_string(c_string* s, size_t start, size_t end) {
  CStringResult result = { .value = NULL, .status = CSTRING_OK };

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

// Return string inside c_string with a null-terminator in case an external function requires it
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

/* Help function used to free the memory used by the c_string** in string_delim */
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

void print(const c_string* s) {
  printf("%.*s", (int)s->length, s->string);
}

void print_delim_strings(c_string** s) {
  size_t i = 0;
  printf("[");
  while (s[i + 1] != NULL) {
    printf("'"); print(s[i]); printf("', ");
    i += 1;
  }
  printf("'"); print(s[i]); printf("']");
}

// Supported Colors: Red, Green, Yellow, Blue, Magenta and Cyan
void print_colored(const c_string* s, const char* color) {
  if ((strcmp(color, "red") == 0) || (strcmp(color, "Red")) == 0) {
    printf(ANSI_COLOR_RED "%.*s" ANSI_COLOR_RESET, (int)s->length, s->string);
  } else if ((strcmp(color, "green") == 0) || (strcmp(color, "Green")) == 0) {
    printf(ANSI_COLOR_GREEN "%.*s" ANSI_COLOR_RESET, (int)s->length, s->string);
  } else if ((strcmp(color, "yellow") == 0) || (strcmp(color, "Yellow")) == 0) {
    printf(ANSI_COLOR_YELLOW "%.*s" ANSI_COLOR_RESET, (int)s->length, s->string);
  } else if ((strcmp(color, "blue") == 0) || (strcmp(color, "Blue")) == 0) {
    printf(ANSI_COLOR_BLUE "%.*s" ANSI_COLOR_RESET, (int)s->length, s->string);
  } else if ((strcmp(color, "magenta") == 0) || (strcmp(color, "Magenta")) == 0) {
    printf(ANSI_COLOR_MAGENTA "%.*s" ANSI_COLOR_RESET, (int)s->length, s->string);
  } else if ((strcmp(color, "cyan") == 0) || (strcmp(color, "Cyan")) == 0) {
    printf(ANSI_COLOR_CYAN "%.*s" ANSI_COLOR_RESET, (int)s->length, s->string);
  } else if ((strcmp(color, "white") == 0) || (strcmp(color, "White")) == 0) {
    printf(ANSI_COLOR_WHITE "%.*s" ANSI_COLOR_RESET, (int)s->length, s->string);
  }
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
//  * If delimiter size > input size, we consider that the delimiter was not found
//  and return the original input.
//  * If delimiter is exactly the input string, we return ['', ''] since we have nothing "before"
//  and "after" the delimiter match.
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
        CStringResult slice = string_from_char(s->string + last_location, (int)(i - last_location));
        if (slice.status != CSTRING_OK) {
          destroy_delim_string(new_split_string);
          return NULL;
        }
        new_split_string[result_index] = slice.value;
      } else {
        // There's nothing before the matched delimiter, so we add an empty string.
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
    CStringResult tail = string_from_char(s->string + last_location, (int)(s->length - last_location));
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
  CStringResult result = { .value = NULL, .status = CSTRING_OK };

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

  // If the input string does not contain the target character, return a copy of the
  // input string.
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
        memcpy(result_string->string + copy_position, s->string + last_location, i - last_location);
        copy_position += i - last_location;
      }

      // Increment last_location even if current index matches to avoid trailing copies.
      last_location = i + 1;
    }
  }

  if (last_location < s->length) {
    memcpy(result_string->string + copy_position, s->string + last_location, s->length - last_location);
  }

  result.value = result_string;
  return result;
}

CStringResult to_lower(const c_string* s);

CStringResult int_to_string(int x) {
  CStringResult result = { .value = NULL, .status = CSTRING_OK };

  int length = snprintf(NULL, 0, "%d", x);
  if (length < 0) {
    result.status = CSTRING_ERR_INTERNAL;
    return result;
  }

  size_t buffer_size = (size_t)length + 1;
  char* s = malloc(buffer_size);
  if (!s) {
    result.status = CSTRING_ERR_NO_MEMORY;
    return result;
  }

  if (!itoa_c(x, s, 10)) {
    free(s);
    result.status = CSTRING_ERR_INTERNAL;
    return result;
  }

  CStringResult new_string = string_from_char(s, length);
  free(s);

  return new_string;
}

// TODO: Check resources on how this can be implemented
CStringResult float_to_string(double x) {
  CStringResult result = { .value = NULL, .status = CSTRING_OK };

  int len = snprintf(NULL, 0, "%g", x);
  if (len < 0) {
    result.status = CSTRING_ERR_INTERNAL;
    return result;
  }

  size_t buffer_size = (size_t)len + 1;
  char* s = malloc(buffer_size);
  if (!s) {
    result.status = CSTRING_ERR_NO_MEMORY;
    return result;
  }

  if (snprintf(s, buffer_size, "%g", x) < 0) {
    free(s);
    result.status = CSTRING_ERR_INTERNAL;
    return result;
  }

  CStringResult new_string = string_from_char(s, len);
  free(s);

  return new_string;
}

const char *cstring_status_str(CStringStatus status) {
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
