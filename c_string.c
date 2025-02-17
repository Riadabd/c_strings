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

enum {
  COPY_ERROR,
  OUT_OF_MEMORY
};

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
  while(ptr1 < ptr) {
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
c_string* initialize_buffer(size_t length) {
  c_string* data = calloc(1, sizeof(c_string));
  if (!data) {
    fputs("Memory allocation failure", stderr);
    exit(EXIT_FAILURE);
  }
  data->length = length;
  data->string = malloc(length);
  if (!(data->string)) {
    fputs("Memory allocation failure", stderr);
    exit(EXIT_FAILURE);
  }

  return data;
}

// Copy contents of a c_string into a new one ("Copy Constructor")
c_string* string_new(const c_string* s) {
  c_string* new_s = calloc(1, sizeof(c_string));
  if (!new_s) {
    fputs("Memory allocation failure", stderr);
    exit(EXIT_FAILURE);
  }

  new_s->length = s->length;
  new_s->string = calloc(1, new_s->length);
  if (!(new_s->string)) {
    fputs("Memory allocation failure", stderr);
    exit(EXIT_FAILURE);
  }
  memcpy(new_s->string, s->string, new_s->length);

  return new_s;
}

c_string* string_from_char(const char* s, const int length) {
  c_string* new_s = calloc(1, sizeof(c_string));
  if (!new_s) {
    fputs("Memory allocation failure", stderr);
    exit(EXIT_FAILURE);
  }

  if (length == 0) {
    new_s->string = NULL;
    new_s->length = 0;
    return new_s;
  }

  new_s->length = length;
  new_s->string = calloc(1, new_s->length);
  if (!(new_s->string)) {
    fputs("Memory allocation failure", stderr);
    exit(EXIT_FAILURE);
  }
  memcpy(new_s->string, s, new_s->length);

  return new_s;
}

// Start and End are inclusive bounds
c_string* sub_string(c_string* s, size_t start, size_t end) {
  if ((start > s->length - 1) || (start < 0) || (end > s->length - 1) || (end < 0)) {
    fputs("Start or end indices are invalid", stderr);
    return NULL;
  }

  if (start > end) {
    fputs("Start index cannot be greater than the end index", stderr);
    return NULL;
  }

  size_t length = end - start + 1;
  c_string* result = calloc(1, sizeof(c_string));
  if (!result) {
    fputs("Memory allocation failure", stderr);
    exit(EXIT_FAILURE);
  }

  result->length = length;
  result->string = malloc(length);
  if (!(result->string)) {
    fputs("Memory allocation failure", stderr);
    exit(EXIT_FAILURE);
  }

  memcpy(result->string, s->string + start, length);

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
  printf("%.*s", s->length, s->string);
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
    printf(ANSI_COLOR_RED "%.*s" ANSI_COLOR_RESET, s->length, s->string);
  } else if ((strcmp(color, "green") == 0) || (strcmp(color, "Green")) == 0) {
    printf(ANSI_COLOR_GREEN "%.*s" ANSI_COLOR_RESET, s->length, s->string);
  } else if ((strcmp(color, "yellow") == 0) || (strcmp(color, "Yellow")) == 0) {
    printf(ANSI_COLOR_YELLOW "%.*s" ANSI_COLOR_RESET, s->length, s->string);
  } else if ((strcmp(color, "blue") == 0) || (strcmp(color, "Blue")) == 0) {
    printf(ANSI_COLOR_BLUE "%.*s" ANSI_COLOR_RESET, s->length, s->string);
  } else if ((strcmp(color, "magenta") == 0) || (strcmp(color, "Magenta")) == 0) {
    printf(ANSI_COLOR_MAGENTA "%.*s" ANSI_COLOR_RESET, s->length, s->string);
  } else if ((strcmp(color, "cyan") == 0) || (strcmp(color, "Cyan")) == 0) {
    printf(ANSI_COLOR_CYAN "%.*s" ANSI_COLOR_RESET, s->length, s->string);
  } else if ((strcmp(color, "white") == 0) || (strcmp(color, "White")) == 0) {
    printf(ANSI_COLOR_WHITE "%.*s" ANSI_COLOR_RESET, s->length, s->string);
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

// Split string according to given delimiter
c_string** string_delim(const c_string* s, const char* delim) {
  // Count number of delimiter occurence
  size_t counter = 0;
  const size_t delim_size = strlen(delim);
  if (s->length >= delim_size) {
    for(size_t i = 0; i <= s->length - delim_size;) {
      if ((memcmp(s->string + i, delim, delim_size) == 0)) {
        counter += 1;
        i += delim_size;
      } else {
        i += 1;
      }
    }
  }

  // Allocate space for the different strings
  if (counter == 0) {
    c_string** new_split_string = calloc(2, sizeof(c_string*));
    if (!new_split_string) {
      fputs("Memory allocation failure", stderr);
      exit(EXIT_FAILURE);
    }

    new_split_string[0] = string_from_char(s->string, s->length);
    new_split_string[1] = NULL;
    return new_split_string;
  }

  // Get delimiter positions
  long long* positions = calloc(counter + 2, sizeof(long long));
  if (!positions) {
    fputs("Memory allocation failure", stderr);
    exit(EXIT_FAILURE);
  }

  positions[0] = -delim_size;
  positions[counter + 1] = s->length;

  size_t secondCounter = 1;
  for(size_t i = 0; i <= s->length - delim_size;) {
    if ((memcmp(s->string + i, delim, delim_size) == 0)) {
      positions[secondCounter] = i;
      secondCounter += 1;
      i += delim_size;
    } else {
      i += 1;
    }
  }

  c_string** new_split_string = calloc(counter + 2, sizeof(c_string*));
  if (!new_split_string) {
    fputs("Memory allocation failure", stderr);
    exit(EXIT_FAILURE);
  }

  // Dummy Node for termination when printing
  new_split_string[counter + 1] = NULL;

  for(size_t i = 1; i <= counter + 1; i++) {
    new_split_string[i - 1] = string_from_char(s->string + positions[i - 1] + delim_size, positions[i] - positions[i - 1] - delim_size);

    if (positions[i] - positions[i - 1] - delim_size == 0) {
      new_split_string[i - 1]->string = NULL;
      new_split_string[i - 1]->length = 0;
      continue;
    }
  }

  free(positions);
  return new_split_string;
}

c_string* trim_char(const c_string* s, const char c) {
  size_t copy_position = 0;
  size_t num_of_occurences = 0;
  size_t location = 0;

  // Count number of character occurences
  for (size_t i = 0; i < s->length; i++) {
    if (s->string[i] == c) {
      num_of_occurences += 1;
    }
  }

  if (num_of_occurences == 0) {
    return string_new(s);
  }

  c_string* result_string = initialize_buffer(s->length - num_of_occurences);

  size_t* positions = calloc(num_of_occurences, sizeof(size_t));
  if (!positions) {
    fputs("Memory allocation failure", stderr);
    exit(EXIT_FAILURE);
  }

  for(size_t i = 0; i < s->length; i++) {
    if (s->string[i] == c) {
      positions[location] = i;
      location += 1;
    }
  }

  if (positions[0] != 0 && (positions[1] - positions[0] != 1)) {
    memcpy(result_string->string, s->string, positions[0]);
    copy_position = positions[0];
  }

  for(size_t i = 0; i <= num_of_occurences - 2; i++) {
    if (positions[i + 1] - positions[i] == 1) {
      continue;
    }

    memcpy(result_string->string + copy_position, s->string + positions[i] + 1, positions[i + 1] - positions[i] - 1);
    copy_position += positions[i + 1] - positions[i] - 1;
  }

  if (positions[num_of_occurences - 1] != s->length - 1) {
    memcpy(result_string->string + copy_position, s->string + positions[num_of_occurences - 1] + 1, s->length - positions[num_of_occurences - 1] - 1);
  }

  free(positions);
  return result_string;
}

c_string* to_lower(const c_string* s);

c_string* int_to_string(int x) {
  int length = snprintf(NULL, 0, "%d", x);
  char* s = malloc(length);
  if (!s) {
    fputs("Memory allocation failure", stderr);
    exit(EXIT_FAILURE);
  }
  s = itoa_c(x, s, 10);

  c_string* result = string_from_char(s, length);
  free(s);
  return result;
}

// TODO: Check resources on how this can be implemented
c_string* float_to_string(double x) {
  int len = snprintf(NULL, 0, "%g", x);
  char* s = (char*) malloc(len + 1);
  if (!s) {
    fputs("Memory allocation failure", stderr);
    exit(EXIT_FAILURE);
  }
  snprintf(s, len + 1, "%g", x);

  c_string* result = calloc(1, sizeof(c_string));
  if (!result) {
    fputs("Memory allocation failure", stderr);
    exit(EXIT_FAILURE);
  }
  create_string(result, len, s);
  free(s);

  return result;
}
