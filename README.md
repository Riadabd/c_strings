# c_strings
c_strings is a C library for dealing with and manipulating strings. It tries to remove some of the pain points associated with C string operations and the possible logic errors and/or memory leaks that might occur.

# Compatibility with char*
c_strings provides `create_string(c_string*, char*)` and `get_null_terminated_string(c_string*)` to transform a string into `c_string*` and `char*`, respectively. This allows the library to be used in contexts where a traditional null-terminated string is required.

# Example Usage
TODO

# Potential Improvements

- [] Add UTF-8 support
- [] Fuzz the codebase
- [] Experiment with arenas (this will help avoid `malloc`, `calloc` and `free` calls for every single (de-)allocation)