/*
 * c_string.h
 * 
 * Copyright 2020 Riad Abdallah <riad.abdallah@hotmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ANSI Escape Codes for colours
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

enum {
	COPY_ERROR,
	OUT_OF_MEMORY
};

typedef struct {
	char* string;
	size_t length;
} c_string;

/* Create string from an input */
void create_string(c_string* s, char* input);
/* Create string given the length of the input if the input is not null-terminated */
void create_string_from_length(c_string* s, char* input, size_t length);
/* Initialize string buffer */
c_string* initialize_buffer(size_t length);
/* Copy contents of a c_string into a new one ("Copy Constructor") */
c_string* string_new(const c_string* s);
c_string* string_from_char(const char* s, const int length);
// Start and End are inclusive bounds
c_string* subString(c_string* s, size_t start, size_t end);

/* Free the string's content and the string itself */
void destroy_string(c_string* input);
/* Help function used to free the memory used by the c_string** in string_delim */
void destroy_delim_string(c_string** s);

/* Concatenate input into string s */
void string_concat(c_string* s, const char* input);
void string_modify(c_string* s, const char* input);

/* Check if two strings are equal
   Longer string is considered greater
   Return > 0 if first is greater than second
   Return 0 if they are equal
   Return < 0 if second is greater than first */
int string_compare(const c_string* first, const c_string* second);

void print(const c_string* s);
void print_delim_strings(c_string** s);

/* Read stdin input into c_string */
void scan_string(c_string* s, const char* prompt);

/* Split string according to given delimiter */
c_string** string_delim(const c_string* s, const char* delim);
c_string* trim_char(const c_string* s, const char c);

c_string* int_to_string(int x);
// TODO: Check resources on how this can be implemented
c_string* float_to_string(float x);