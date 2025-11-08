# c_strings

c_strings is a C library for dealing with and manipulating strings. It tries to remove some of the pain points associated with C string operations and the possible logic errors and/or memory leaks that might occur.

# Compatibility with char*

c_strings provides `create_string(c_string*, char*)` and `get_null_terminated_string(c_string*)` to transform a string into `c_string*` and `char*`, respectively. This allows the library to be used in contexts where a traditional null-terminated string is required.

# Example Usages

## String creation and mutation

Below is a minimal program that demonstrates the typical lifecycle of a `c_string`: creation, mutation, conversion, and cleanup. All routines return either `void` or a `CStringResult` that should be checked for `status == CSTRING_OK` before using the returned value.

```c
#include "c_string.h"
#include <stdio.h>

int main(void) {
    c_string name = {0};

    // Create from an existing buffer (does not copy input; manage lifetime yourself).
    char raw[] = " Alice ";
    create_string(&name, sizeof(raw) - 1, raw);

    // Modify in place and concatenate new data.
    string_modify(&name, "Alice");
    string_concat(&name, " Smith");

    // Convert while preserving storage guarantees.
    CStringResult lower = to_lower(&name);
    if (lower.status == CSTRING_OK) {
        printf("lowercase: %s\n", get_null_terminated_string(lower.value));
        destroy_string(lower.value);
    }

    // Pretty-print with color support.
    print_colored(&name, "Green");

    destroy_string(&name);
    return 0;
}
```

## Slicing and delimiters

The next example focuses on slicing a string and iterating over delimiter-separated tokens.

```c
#include "c_string.h"
#include <stdio.h>

static void print_token(c_string* token, size_t index) {
    printf("token[%zu]=%s\n", index, get_null_terminated_string(token));
}

int main(void) {
    CStringResult sentence = string_from_char("Hello,World,UTF-8", 18);
    if (sentence.status != CSTRING_OK) return 1;

    // Extract the first word via byte indices (inclusive).
    CStringResult hello = sub_string_checked(sentence.value, 0, 4);
    if (hello.status == CSTRING_OK) {
        printf("prefix=%s\n", get_null_terminated_string(hello.value));
        destroy_string(hello.value);
    }

    // Split the sentence at commas and inspect each token.
    c_string** tokens = string_delim(sentence.value, ",");
    size_t count = get_delim_string_length(tokens);
    for (size_t i = 0; i < count; ++i) {
        print_token(tokens[i], i);
        destroy_string(tokens[i]);
    }
    destroy_delim_string(tokens);

    destroy_string(sentence.value);
    return 0;
}
```


# Potential Improvements

- [x] Add tests
- [x] Add a fuzzing harness using `afl++` and fuzz the codebase
- [x] Add UTF-8 support
- [ ] Experiment with arenas (this will help avoid `malloc`, `calloc` and `free` calls for every single (de-)allocation)
- [ ] Implement a mini-regex engine

## UTF-8 Lowercasing via utf8proc

`tolower` from the C standard library only understands single-byte characters in the active locale. If you need a Unicode-aware `to_lower` while using this repo, a good option is [utf8proc](https://juliastrings.github.io/utf8proc/). It is MIT-licensed, ~250 KB of tables, and ships as a single C file you can vendor or link against.

### Building with utf8proc

1. Clone or download utf8proc somewhere in your project tree (e.g. `deps/utf8proc`).
2. Add the following to your `Makefile` (adjust paths as needed):
   ```make
   CFLAGS += -I$(PROJECT_ROOT)/deps/utf8proc
   LDLIBS += $(PROJECT_ROOT)/deps/utf8proc/utf8proc.c
   ```
   Because utf8proc has no external dependencies, compiling the `.c` file directly keeps the build self-contained. If you prefer building a static lib, replace the last line with `$(PROJECT_ROOT)/deps/utf8proc/libutf8proc.a`.

### Example adapter

The snippet below shows how to wrap utf8proc so the rest of this library can keep using `CStringResult`. It relies on `utf8proc_map`, which returns freshly allocated UTF-8 bytes already case-folded.

```c
#include <utf8proc.h>

CStringResult to_lower_utf8proc(const c_string* s) {
  CStringResult result = {.value = NULL, .status = CSTRING_OK};

  if (!s || !s->string) {
    result.status = CSTRING_ERR_INVALID_ARG;
    return result;
  }

  utf8proc_uint8_t* lowered = NULL;
  utf8proc_ssize_t lowered_len = utf8proc_map(
      (const utf8proc_uint8_t*)s->string, (utf8proc_ssize_t)s->length,
      &lowered, UTF8PROC_STABLE | UTF8PROC_COMPOSE | UTF8PROC_CASEFOLD);

  if (lowered_len < 0 || !lowered) {
    result.status = CSTRING_ERR_INTERNAL;
    return result;
  }

  CStringResult wrapped = string_from_char((const char*)lowered, (size_t)lowered_len);
  utf8proc_free(lowered);
  return wrapped;
}
```

Notes:

- `UTF8PROC_CASEFOLD` performs locale-independent lowercasing (Unicode CaseFolding). Add `UTF8PROC_STRIPMARK` or other flags if you need additional normalization.
- `utf8proc_map` allocates with `malloc`; always free the buffer with `utf8proc_free` once it has been copied into a `c_string`.
- If you need Turkish/Azeri locale-specific dotted/dotless-I behavior, call `utf8proc_map_custom` with a tailored table or post-process the output accordingly.

# Development

- `make lib` builds `out/libc_strings.a` for reuse in other binaries.
- `make test` runs the Unity/Ceedling test suite located in `tests/`.

## Fuzzing

- **Platform support**: The fuzzing harness is maintained on macOS only right now.
- **Prerequisites**: Install Homebrew and then `brew install llvm` so `afl-clang-fast` is available. Ensure afl++ itself is installed and on your `PATH` (`afl-fuzz --version`). You’ll also need Xcode Command Line Tools for `make`, plus a recent Ruby if you plan to use the helper Fish scripts.
- **Build the harness**: Run `make fuzz-build` to compile `fuzz/c_string_fuzzer.c` together with the library under afl++ with AddressSanitizer. The instrumented binary is written to `out/fuzz/c_string_fuzzer`. Rebuild after modifying library code.
- **Run a session**: Execute `make fuzz` to launch afl++ with the bundled seeds in `fuzz/corpus/`, storing results under `fuzz/findings/`. The target wraps the required macOS environment variables (`MallocNanoZone=0`, `AFL_SKIP_CPUFREQ=1`) so you don’t have to remember them.
    - **Resume a session**: You can a resume a fuzzing session you had previously stopped by running `make fuzz-resume` or `scripts/run_afl_session.fish make fuzz` as discussed below.
- **If afl++ complains**: Use `scripts/run_afl_session.fish make fuzz` to apply temporary macOS CrashReporter and SHM tweaks before starting the run; the script reverts them once the session ends.
    - SHM stands for shared memory. For more deeper info, afl++ uses a System V shared-memory segment to hand coverage data from the instrumented target back to the fuzzer. On macOS the default limits for that SHM region can be tight, so the helper script tweaks those limits before launching afl-fuzz and restores them when the run ends.
    - macOS's crashreporter is also temporarily disabled as advised (afl++ will refuse to run in any case before you do so). The reason is that afl++ collects crashes in an output file, which macOS would otherwise trigger the reporter GUI for and distrupt the running process.

## Testing

The steps below highlight the needed steps to add tests to a C codebase using unity + Ceedling:
1. Install Ceedling (`gem install ceedling`).
    * On macOS, you will need to install ruby through homebrew (`brew install ruby`) in order to use ruby3 instead of the bundled ruby2.
2. Generate the test harness inside the repo root with `ceedling new tests` (creates `tests/`).
3. Update `tests/project.yml` to include the production sources (`../c_string.c`) and headers (add `../` to the include paths).
4. Add Unity test files under `tests/test/` (e.g., `test_string_delim.c`, `test_trim_char.c`).
5. Run the suite with `make test` (invokes `ceedling test:all`).

For the purposes of running this repo's tests, you only need to install Ceedling using `gem` and then run `make test` (make sure to have `ruby3` installed).
