/*
 * c_strings.h
 * 
 * Copyright 2017 Riad Abdallah <riad.abdallah@hotmail.com>
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
#include <assert.h>
#include <string.h>
#include <stdarg.h>

// ANSI Escape Codes for colours
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define INT_ADD_OVERFLOW_P(a, b)                                       \
   __builtin_add_overflow_p (a, b, (__typeof__ ((a) + (b))) 0)



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
/* Free the string's content and the string itself */
void destroy_string(c_string* input);
/* Free the memory allocated for a char* by the stack allocated string */
void destroy_local_string(c_string* input);
/* Concatenate input into string s */
void string_concat(c_string* s, const char* input);

void string_modify(c_string* s, const char* input);
int string_copy(c_string* dest, const c_string* src);
void actual_print(const c_string* s, const char* message, va_list argp);
void print(const c_string* s, const char* message, ...);
/* Read stdin input into c_string */
void scan_string(c_string* s, const char* prompt);
/* Split string according to given delimiter */
c_string** string_delim(const c_string* s, const char* delim);
c_string* trim_char(const c_string* s, const char c);
/* Help function to pretty print the result of string_delim */
void pretty_print(c_string** s);
/* Help function used to free the memory used by the c_string** in string_delim */
void destroy_delim_string(c_string** s);


c_string* read_from_file(const char* filename);