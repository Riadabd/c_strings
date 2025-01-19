#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* String Struct Definition */

typedef struct {
  char* string;
  size_t length;
} c_string;

// Create string from an input
void create_string(c_string* s, size_t length, char* input);

// Initialize string buffer
c_string* initialize_buffer(size_t length);

// Copy contents of a c_string into a new one ("Copy Constructor")
c_string* string_new(const c_string* s);

c_string* string_from_char(const char* s, const int length);

// Start and End are inclusive bounds
c_string* sub_string(c_string* s, size_t start, size_t end);

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

c_string* trim_char(const c_string* s, const char c);

c_string* to_lower(const c_string* s);

c_string* int_to_string(int x);

// TODO: Check resources on how this can be implemented
c_string* float_to_string(double x);
