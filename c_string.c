/*
 * c_strings.c
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
 * clang -O1 -g -fsanitize=undefined -fno-omit-frame-pointer c_strings.c main.c -o test
 * gcc -Wall -Wextra c_strings.c main.c -o test
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
        r++;

    return r;
}

// To be used by stack-allocated c_strings
void init_local_string(c_string* s) {
	s->length = 0;
	s->string = NULL;
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

c_string* initialize_buffer(size_t length) {
	c_string* data = calloc(1, sizeof(c_string));
	data->length = length;
	data->string = malloc(length);
	if (!data->string) {
		fputs("Memory allocation failure", stderr);
		exit(EXIT_FAILURE);
	}
	return data;
}

void destroy_string(c_string* input) {
	free(input->string);
	free(input);
	// ASK: Is it good practice to set to NULL after freeing?
}

void destroy_local_string(c_string* input) {
	free(input->string);
	// ASK: Is it good practice to set to NULL after freeing?
}

void string_concat(c_string* s, const char* input) {
	// TODO: Check how to handle possible allocation size overflow
	s->string = realloc(s->string, s->length + strlen(input));
	memcpy(s->string + s->length, input, strlen(input));
	s->length += strlen(input);
}

int string_copy(c_string* dest, const c_string* src) {
	dest = calloc(1, sizeof(c_string));
	create_string_from_length(dest, src->string, src->length);

	return 0;
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

static char* int_to_string(int x) {
	int length = snprintf( NULL, 0, "%d", x );
	char* str = malloc(length + 1);
	snprintf( str, length + 1, "%d", x );
	return str;
}

static void subs_string(const c_string* s, c_string* new, const size_t positions[], const size_t num_elements, const char* message, const size_t length) {
	size_t pointer = 0;
	size_t last_pos = 0;
	size_t i = 0;
	if (positions[0] == 0) {
		memcpy(new->string, s->string, s->length);
		pointer = s->length;
		i = 1;
		last_pos = positions[0];
		// If there's only one '{}', we need to copy the remaining part.
		if (num_elements == 1) {
			memcpy(new->string + pointer, message + 2, length - 2);
		}
	}
	else {
		memcpy(new->string, message, positions[i]);
		pointer += positions[i];
		memcpy(new->string + pointer, s->string, s->length);
		pointer += s->length;
		i = 1;
		last_pos = positions[0];
		if (num_elements == 1) {
			memcpy(new->string + pointer, message + positions[0] + 2, length - positions[0] - 2);
		}
	}
	while(i < num_elements) {
		memcpy(new->string + pointer, message + positions[i - 1] + 2, positions[i] - last_pos - 2);
		pointer += positions[i] - last_pos - 2;
		memcpy(new->string + pointer, s->string, s->length);
		pointer += s->length;
		last_pos = positions[i];
		++i;
		if (i == num_elements) {
			memcpy(new->string + pointer, message + positions[i - 1] + 2, length - positions[i - 1] - 2);
		}
	}
}

static void initialize_string(c_string* s, size_t length) {
	s->string = malloc(length + 1);
	memset(s->string, 0, length);
	s->string[length] = '\0';
}

// Message should include '{}' to signify where the string needs to be printed	
void actual_print(const c_string* s, const char* message, va_list argp) {
	// NOTE: Don't use function composition inside (f)printf
	c_string local = {NULL, 0};
	string_modify(&local, message);
	const size_t size = (local.length % 2 == 0) ? (local.length / 2) : ((local.length - 1) / 2);
	size_t positions[size]; /* Undefined behavior? */
	size_t counter = 0;
	size_t num_of_digits = count_bchop(s->length);
	// Find instances of '{}'
	for(size_t i = 0; i < local.length - 1; i++) {
		if (local.string[i] == '{' && local.string[i + 1] == '}') {
			positions[counter] = i;
			++counter;
			++i;
		} 
	}
	if (counter == 0) {
		vfprintf(stdout, message, argp);
		destroy_local_string(&local);
		return;
	}
	// Modify the message string to pass it to fprintf
	char* num_digits = int_to_string(s->length);
	size_t length_diff = (s->length > 2) ? (s->length - 2) : (2 - s->length);
	c_string new = {NULL, local.length + (counter) * (length_diff)};
	
	initialize_string(&new, new.length);
	subs_string(s, &new, positions, counter, message, local.length);
	// NOTE: A null-terminator is added in this specific case due to strchrnul() returning a pointer to the non-existant null-terminator.
	// ASK: Would not adding a null-terminator in this case raise a security concern due to having a pointer pointing beyond the allocated memory?
	vfprintf(stdout, new.string, argp);
	destroy_local_string(&new);
	
	destroy_local_string(&local);
	free(num_digits);
}

void print(const c_string* s, const char* message, ...) {
	va_list argp;
	va_start(argp, message);
	actual_print(s, message, argp);
	va_end(argp);
}

void scan_string(c_string* s, const char* prompt) {
	
}

c_string** string_delim(const c_string* s, const char* delim) {
	
	// Count number of delimiter occurence
	// TODO: Write or use existing vector implementation to immediately add positions rather than use a separate for-loop
	size_t counter = 0;
	const size_t delim_size = strlen(delim);
	if (s->length >= delim_size) {
		for(size_t i = 0; i <= s->length - delim_size; i++) {
			if ((memcmp(s->string + i, delim, delim_size) == 0)) {
				printf("i is %zu.\n", i);
				i += delim_size - 1;
				++counter;
			}
		}
	}
	
	// Allocate space for the different strings
	if (counter == 0) {
		c_string** new_split_string = calloc(1, sizeof(c_string*));
		new_split_string[0] = calloc(1, sizeof(c_string));
		//new_split_string[1] = calloc(1, sizeof(c_string));
		c_string* input = new_split_string[0];
		create_string_from_length(input, s->string, s->length);
		//create_string_from_length(new_split_string[1], NULL, 0);
		return new_split_string;	
	}
	
	// Get delimiter positions 
	size_t positions[counter]; /* VLA: Undefined behavior? */
	const size_t array_length = counter;
	
	// Re-used variable: Watch out for implications of reusage!
	counter = 0;
	for(size_t i = 0; i <= s->length - delim_size; i++) {
		if ((memcmp(s->string + i, delim, delim_size) == 0)) {
			positions[counter] = i;
			i += delim_size - 1;
			++counter;	
		}
	}
	
	// Determine how many strings there are depending on delimiter positions
	size_t possible_strings = array_length + 1;
	size_t jump = 0;
	if (positions[0] == 0) {
		--possible_strings;
		jump = delim_size;
	}
	if (positions[array_length - 1] >= s->length - delim_size) {
		--possible_strings;
		jump = 0;
	}
	c_string** new_split_string = calloc(possible_strings + 1, sizeof(c_string*));
	for(size_t i = 0; i < possible_strings + 1; i++) {
		new_split_string[i] = calloc(1, sizeof(c_string));
	}

	if (possible_strings == 1) {
		create_string_from_length(new_split_string[0], s->string + jump, s->length - delim_size);
		create_string_from_length(new_split_string[1], NULL, 0);
		//free(positions);
		return new_split_string;
	}

	size_t index = 0; /* To navigate the new_split_string array */
	if (positions[0] != 0) {
		create_string_from_length(new_split_string[index], s->string, positions[0]);
		++index;	
	} 
	
	for(size_t i = 0; i <= array_length - 2; i++) {
		create_string_from_length(new_split_string[index], s->string + positions[i] + delim_size, positions[i + 1] - positions[i] - delim_size);
		++index;
	}
	
	if (positions[array_length - 1] != s->length - delim_size) {
		create_string_from_length(new_split_string[index], s->string + positions[array_length - 1] + delim_size, s->length - positions[array_length - 1] - delim_size);
		++index;
	}
	create_string_from_length(new_split_string[index], NULL, 0);
	
	printf("There is(are) %lu delimiter(s).\n", counter);
	//free(positions);
	return new_split_string;
}

void pretty_print(c_string** s) {
	size_t i = 0;
	printf("[");
	while (s[i]->string != NULL) {
		if (s[i + 1]->string != NULL) {
			print(s[i], "{}, ");
		}
		else {
			print(s[i], "{}]\n");
		}
		++i;
	}
}

void destroy_delim_string(c_string** s) {
	size_t i = 0;
	while (s[i]->string != NULL) {
		destroy_string(s[i]);
		++i;
	}
	destroy_string(s[i]);
	free(s);
}

c_string* trim_char(const c_string* s, const char c) {
	c_string* result_string = initialize_buffer(s->length);

	size_t start = 0, end = 0;
	size_t result_position = 0;
	size_t length = 0;

	if (s->string[0] != c) {
		start = 0;
	}
	else {
		start = 1;
	}

	for (size_t i = 1; i < s->length; i++) {
		if (s->string[i] != c) {
			if (start > end) {
				end = start - 1;
			}
			end++;
		}
		else {
			if (start <= end) {
				length += end - start + 1;
				memcpy(result_string->string + result_position, s->string + start, end - start + 1);
				result_position += end - start + 1;
			}
			start = i + 1;
		}
	}

	// In case there are no delimiters at the end
	if (start < s->length) {
		memcpy(result_string->string + result_position, s->string + start, end - start + 1);
		length += end - start + 1;
	}

	c_string* trimmed_string = malloc(sizeof(c_string));
	create_string_from_length(trimmed_string, result_string->string, length);

	destroy_string(result_string);

	return trimmed_string;
}

static unsigned long get_file_length(FILE *ifp) {
   unsigned long file_length;

   fseek(ifp, 0, SEEK_END);
   file_length = ftell(ifp);
   rewind(ifp);
	
   return file_length;
}

c_string* read_from_file(const char* filename) {
	FILE* fp = fopen(filename, "r");
	if (!fp) {
		fputs("Memory allocation failure", stderr);
		exit(EXIT_FAILURE);
	}
	
	const unsigned long file_length = get_file_length(fp);
	c_string* data = initialize_buffer(file_length);
	fread(data->string, 1, file_length, fp);
	fclose(fp);

	return data;
}
