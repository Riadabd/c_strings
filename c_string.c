/*
 * c_string.c
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
 * clang -O1 -g -fsanitize=undefined -fno-omit-frame-pointer c_string.c main.c -o test
 * gcc -Wall -Wextra c_string.c main.c -o test
 * 
 */

#include "c_string.h"

// Count number of digits
static unsigned int count_bchop (unsigned int n) {
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
    if (n >= 10)
        r += 1;

    return r;
}

// Convert integer value to char*
static char* itoa_c(int value, char* result, int base) {
    // check that the base if valid
    if (base < 2 || base > 36) { result = NULL; return result; }

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

c_string* int_to_string(int x) {
	int length = snprintf(NULL, 0, "%d", x);
	char* s = malloc(length);
	s = itoa_c(x, s, 10);

	c_string* result = string_from_char(s, length);
	free(s);
	return result;
}

void create_string(c_string* s, char* input) {
	size_t length = strlen(input);
	s->string = malloc(length);
	if (!s->string) {
		fputs("Memory allocation failure", stderr);
		exit(EXIT_FAILURE);
	}
	s->length = length;
	memcpy(s->string, input, s->length);
}

void create_string_from_length(c_string* s, char* input, size_t length) {
	if (input != NULL) {
		s->string = malloc(length);
		if (!s->string) {
			fputs("Memory allocation failure", stderr);
			exit(EXIT_FAILURE);
	}
		s->length = length;
		memcpy(s->string, input, s->length);
	}
	else {
		s->string = NULL;
		s->length = 0;
	}
}

c_string* string_new(const c_string* s) {
	c_string* new_s = calloc(1, sizeof(c_string));
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
	new_s->length = length;
	new_s->string = calloc(1, new_s->length);
	if (!(new_s->string)) {
		fputs("Memory allocation failure", stderr);
		exit(EXIT_FAILURE);
	}
	memcpy(new_s->string, s, new_s->length);

	return new_s;
}

c_string* initialize_buffer(size_t length) {
	c_string* data = calloc(1, sizeof(c_string));
	data->length = length;
	data->string = malloc(length);
	if (!(data->string)) {
		fputs("Memory allocation failure", stderr);
		exit(EXIT_FAILURE);
	}

	return data;
}

c_string* subString(c_string* s, size_t start, size_t end) {
	size_t length = end - start + 1;
	c_string* result = calloc(1, sizeof(c_string));
	result->length = length;
	result->string = malloc(length);
	memcpy(result->string, s->string + start, length);

	return result;
}

void destroy_string(c_string* input) {
	free(input->string);
	free(input);
}

void string_concat(c_string* s, const char* input) {
	// TODO: Check how to handle possible allocation size overflow
	s->string = realloc(s->string, s->length + strlen(input));
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

int string_compare(const c_string* first, const c_string* second) {
	if (first->length > second->length) {
		return 1;
	}
	else if (first->length < second->length) {
		return -1;
	}

	return memcmp(first->string, second->string, first->length);
}

c_string** string_delim(const c_string* s, const char* delim) {
	// Count number of delimiter occurence
	size_t counter = 0;
	const size_t delim_size = strlen(delim);
	if (s->length >= delim_size) {
		for(size_t i = 0; i <= s->length - delim_size;) {
			if ((memcmp(s->string + i, delim, delim_size) == 0)) {
				counter += 1;
				i += delim_size;
			}
			else {
				i += 1;
			}
		}
	}
	
	// Allocate space for the different strings
	if (counter == 0) {
		c_string** new_split_string = calloc(2, sizeof(c_string*));
		new_split_string[0] = calloc(1, sizeof(c_string));
		new_split_string[1] = NULL;
		create_string_from_length(new_split_string[0], s->string, s->length);
		return new_split_string;
	}
	
	// Get delimiter positions
	long long* positions = calloc(counter + 2, sizeof(long long));
	positions[0] = -delim_size;
	positions[counter + 1] = s->length;
	
	size_t secondCounter = 1;
	for(size_t i = 0; i <= s->length - delim_size;) {
		if ((memcmp(s->string + i, delim, delim_size) == 0)) {
			positions[secondCounter] = i;
			secondCounter += 1;
			i += delim_size;
		}
		else {
			i += 1;
		}
	}
	
	c_string** new_split_string = calloc(counter + 2, sizeof(c_string*));
	for(size_t i = 0; i <= counter; i++) {
		new_split_string[i] = calloc(1, sizeof(c_string));
	}
	// Dummy Node for termination when printing
	new_split_string[counter + 1] = NULL;

	for(size_t i = 1; i <= counter + 1; i++) {
		if (positions[i] - positions[i - 1] - delim_size == 0) {
			new_split_string[i - 1]->string = NULL;
			new_split_string[i - 1]->length = 0;
			continue;
		}
		create_string_from_length(new_split_string[i - 1], s->string + positions[i - 1] + delim_size, positions[i] - positions[i - 1] - delim_size);
	}
	
	free(positions);
	return new_split_string;
}

void destroy_delim_string(c_string** s) {
	size_t i = 0;
	while (s[i] != NULL) {
		destroy_string(s[i]);
		i += 1;
	}

	free(s);
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