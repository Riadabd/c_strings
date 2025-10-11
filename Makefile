OUT_DIR := out
CC ?= gcc
GCC_LINUX_CC ?= /opt/homebrew/bin/gcc-15
CEEDLING ?= /opt/homebrew/lib/ruby/gems/3.4.0/bin/ceedling
LIB_OBJ := $(OUT_DIR)/c_string.o
LIB := $(OUT_DIR)/libc_strings.a

# Compiler flags

## Common

CFLAGS_COMMON = -std=c17 -O2 -g -fstack-protector-strong \
								-Wall -Wextra -Wpedantic \
								-Wconversion -Wshadow -Wnull-dereference \
								-Wdouble-promotion -Wformat=2 -Wimplicit-fallthrough

## Clang

CFLAGS_CLANG_SPECIFIC = -Wcomma -Wunreachable-code-aggressive

CFLAGS_CLANG = $(CFLAGS_COMMON) $(CFLAGS_CLANG_SPECIFIC)

## GCC

# -U_FORTIFY_SOURCE is added ahead of -D_FORTIFY_SOURCE=2 to no longer trigger the macro redefinition warning when the toolchain predefines _FORTIFY_SOURCE.
CFLAGS_GCC_SPECIFIC = -Wduplicated-cond -Wduplicated-branches \
                  -Wtrampolines -Wlogical-op -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2

CFLAGS_GCC = $(CFLAGS_COMMON) $(CFLAGS_GCC_SPECIFIC)

CPU_FLAGS ?= -march=native

## Sanitizers

SANITIZERS ?= address,undefined

ifneq ($(strip $(SANITIZERS)),)
SANITIZE_LIST := $(subst ,, ,$(SANITIZERS))

ifneq (,$(filter address,$(SANITIZE_LIST)))
ifneq (,$(filter thread,$(SANITIZE_LIST)))
$(error Sanitizers 'address' and 'thread' cannot be combined. Set SANITIZERS=thread to use ThreadSanitizer.)
endif
endif

SANITIZE_FLAGS = -fsanitize=$(SANITIZERS)
SANITIZE_CFLAGS = $(SANITIZE_FLAGS) -fno-omit-frame-pointer
else
SANITIZE_CFLAGS =
endif

# Fetch all .c and h. files in the repo except those inside ./out/ and
# ./tests/build/* .
FORMAT_SRCS := $(shell find . -type f \( -name '*.c' -o -name '*.h' \) \
	! -path './out/*' ! -path './tests/build/*')

FUZZ_OUT_DIR := $(OUT_DIR)/fuzz
FUZZ_TARGET := $(FUZZ_OUT_DIR)/c_string_fuzzer
AFL_CC ?= afl-clang-fast
AFL_CFLAGS ?= -std=c99 -Wall -Wextra -pedantic -Wno-gnu-statement-expression -O1 -g

.PHONY: lib test test-one clean fuzz-build fuzz fuzz-resume fmt fmt-check

# This allows the user to run `make clang` and have the Makefile automatically
# use the correct make target.
ifeq ($(OS),Windows_NT)
TARGET_SUFFIX := win
else
TARGET_SUFFIX := linux
endif

.PHONY: clang gcc
clang: clang_$(TARGET_SUFFIX)
gcc: gcc_$(TARGET_SUFFIX)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(LIB_OBJ): c_string.c c_string.h | $(OUT_DIR)
	$(CC) -std=c99 -Wall -Wextra -pedantic -O2 -c c_string.c -o $(LIB_OBJ)

$(LIB): $(LIB_OBJ)
	ar rcs $(LIB) $(LIB_OBJ)

lib: $(LIB)

#
# Fuzzing
#

$(FUZZ_OUT_DIR): | $(OUT_DIR)
	mkdir -p $(FUZZ_OUT_DIR)

$(FUZZ_TARGET): c_string.c c_string.h fuzz/c_string_fuzzer.c | $(FUZZ_OUT_DIR)
	AFL_USE_ASAN=1 $(AFL_CC) $(AFL_CFLAGS) -I. c_string.c fuzz/c_string_fuzzer.c -o $(FUZZ_TARGET)

fuzz-build: $(FUZZ_TARGET)

fuzz: fuzz-build
	env MallocNanoZone=0 AFL_SKIP_CPUFREQ=1 afl-fuzz -i fuzz/corpus -o fuzz/findings -- $(FUZZ_TARGET) @@

fuzz-resume: fuzz-build
	env MallocNanoZone=0 AFL_SKIP_CPUFREQ=1 AFL_AUTORESUME=1 afl-fuzz -i - -o fuzz/findings -- $(FUZZ_TARGET) @@

#
# Compiling
#

clang_win: | $(OUT_DIR)
	clang -std=c99 -Weverything -g -fsanitize=undefined -fno-omit-frame-pointer -march=native c_string.c main.c -o $(OUT_DIR)/clang_win_test.exe

clang_linux: | $(OUT_DIR)
	clang $(CFLAGS_CLANG) $(CPU_FLAGS) $(SANITIZE_CFLAGS) c_string.c main.c -o $(OUT_DIR)/clang_linux_test

gcc_win: | $(OUT_DIR)
	gcc -std=c99 -Wall -Wextra -g -fno-omit-frame-pointer -march=native c_string.c main.c -o $(OUT_DIR)/gcc_win_test.exe

gcc_linux: | $(OUT_DIR)
	$(GCC_LINUX_CC) $(CFLAGS_GCC) $(CPU_FLAGS) $(SANITIZE_CFLAGS) c_string.c main.c -o $(OUT_DIR)/gcc_linux_test

#
# Testing
#

test: lib
	cd tests && $(CEEDLING) test:all

test-one: lib
ifeq ($(strip $(TEST)),)
	$(error TEST variable not set. Usage: make test-one TEST=test_name)
endif
	cd tests && $(CEEDLING) test:$(TEST)

clean:
	rm -rf $(OUT_DIR) tests/build

#
# Formatting
#

fmt:
	@if [ -z "$(FORMAT_SRCS)" ]; then \
		echo "No C/C++ sources found to format."; \
	else \
		clang-format -i $(FORMAT_SRCS); \
	fi

fmt-check:
	@if [ -z "$(FORMAT_SRCS)" ]; then \
		echo "No C/C++ sources found to format."; \
	else \
		clang-format --dry-run --Werror $(FORMAT_SRCS); \
	fi
