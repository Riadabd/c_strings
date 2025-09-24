#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* String Struct Definition */

typedef struct {
  char* string;
  size_t length;
} c_string;

typedef enum {
  CSTRING_OK = 0,
  CSTRING_ERR_NO_MEMORY,
  CSTRING_ERR_INVALID_ARG,
  CSTRING_ERR_IO,
  CSTRING_ERR_OVERFLOW,
  CSTRING_ERR_INTERNAL,
} CStringStatus;

typedef struct {
  c_string* value;
  CStringStatus status;
} CStringResult;

// Create string from an input
void create_string(c_string* s, size_t length, char* input);

// Initialize string buffer
CStringResult initialize_buffer(size_t length);

// Copy contents of a c_string into a new one ("Copy Constructor")
CStringResult string_new(const c_string* s);

CStringResult string_from_char(const char* s, const int length);

// Start and End are inclusive bounds
CStringResult sub_string(c_string* s, size_t start, size_t end);

// Return string inside c_string with a null-terminator in case an external function requires it
char* get_null_terminated_string(c_string* s);

/* Free the string's content and the string itself */
void destroy_string(c_string* input);

/* Help function used to free the memory used by the c_string** in string_delim */
void destroy_delim_string(c_string** s);

/* Concatenate input into string s */
void string_concat(c_string* s, const char* input);

void string_modify(c_string* s, const char* input);

/* Check if two strings are equal.
   Longer string is considered greater.
   Return > 0 if first is greater than second.
   Return 0 if they are equal.
   Return < 0 if second is greater than first. */
int string_compare(const c_string* first, const c_string* second);

/* Printing Functions */

void print(const c_string* s);

void print_delim_strings(c_string** s);

// Supported Colors: Red, Green, Yellow, Blue, Magenta and Cyan
void print_colored(const c_string* s, const char* color);

size_t get_delim_string_length(c_string** s);

// Read stdin input into c_string
void scan_string(c_string* s, const char* prompt);

/* String Transformation Functions */

// Split string according to given delimiter
c_string** string_delim(const c_string* s, const char* delim);

CStringResult trim_char(const c_string* s, const char c);

CStringResult to_lower(const c_string* s);

CStringResult int_to_string(int x);

// TODO: Check resources on how this can be implemented
CStringResult float_to_string(double x);

const char *cstring_status_str(CStringStatus status);
