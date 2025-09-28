# c_strings

c_strings is a C library for dealing with and manipulating strings. It tries to remove some of the pain points associated with C string operations and the possible logic errors and/or memory leaks that might occur.

# Compatibility with char*

c_strings provides `create_string(c_string*, char*)` and `get_null_terminated_string(c_string*)` to transform a string into `c_string*` and `char*`, respectively. This allows the library to be used in contexts where a traditional null-terminated string is required.

# Example Usage

TODO

# Potential Improvements

- [x] Add tests
- [x] Add a fuzzing harness using `afl++` and fuzz the codebase
- [] Add UTF-8 support
- [] Experiment with arenas (this will help avoid `malloc`, `calloc` and `free` calls for every single (de-)allocation)
- [] Implement a mini-regex engine

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
