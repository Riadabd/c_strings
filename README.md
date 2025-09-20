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

# Development

- `make lib` builds `out/libc_strings.a` for reuse in other binaries.
- `make test` runs the Unity/Ceedling suite located in `tests/`.

## Testing

1. Install Ceedling (`gem install ceedling`).
  * On macOS, you will need to install ruby through homebrew (`brew install ruby`) in order to use ruby3 instead of the bundled ruby2.
2. Generate the test harness inside the repo root with `ceedling new tests` (creates `tests/`).
3. Update `tests/project.yml` to include the production sources (`../c_string.c`) and headers (add `../` to the include paths).
4. Add Unity test files under `tests/test/` (e.g., `test_string_delim.c`, `test_trim_char.c`).
5. Run the suite with `make test` (invokes `ceedling test:all`).
