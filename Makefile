OUT_DIR := out
CC ?= gcc
CEEDLING ?= /opt/homebrew/lib/ruby/gems/3.4.0/bin/ceedling
LIB_OBJ := $(OUT_DIR)/c_string.o
LIB := $(OUT_DIR)/libc_strings.a
FORMAT_SRCS := $(shell find . -type f \( -name '*.c' -o -name '*.h' \) \
	! -path './out/*' ! -path './tests/build/*')

FUZZ_OUT_DIR := $(OUT_DIR)/fuzz
FUZZ_TARGET := $(FUZZ_OUT_DIR)/c_string_fuzzer
AFL_CC ?= afl-clang-fast
AFL_CFLAGS ?= -std=c99 -Wall -Wextra -pedantic -Wno-gnu-statement-expression -O1 -g

.PHONY: lib test test-one clean fuzz-build fuzz fuzz-resume fmt fmt-check

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

$(FUZZ_OUT_DIR): | $(OUT_DIR)
	mkdir -p $(FUZZ_OUT_DIR)

$(FUZZ_TARGET): c_string.c c_string.h fuzz/c_string_fuzzer.c | $(FUZZ_OUT_DIR)
	AFL_USE_ASAN=1 $(AFL_CC) $(AFL_CFLAGS) -I. c_string.c fuzz/c_string_fuzzer.c -o $(FUZZ_TARGET)

fuzz-build: $(FUZZ_TARGET)

fuzz: fuzz-build
	env MallocNanoZone=0 AFL_SKIP_CPUFREQ=1 afl-fuzz -i fuzz/corpus -o fuzz/findings -- $(FUZZ_TARGET) @@

fuzz-resume: fuzz-build
	env MallocNanoZone=0 AFL_SKIP_CPUFREQ=1 AFL_AUTORESUME=1 afl-fuzz -i - -o fuzz/findings -- $(FUZZ_TARGET) @@

clang_win: | $(OUT_DIR)
	clang -std=c99 -Weverything -g -fsanitize=undefined -fno-omit-frame-pointer -march=native c_string.c main.c -o $(OUT_DIR)/clang_win_test.exe

clang_linux: | $(OUT_DIR)
	clang -std=c99 -Weverything -g -fsanitize=address,undefined -fno-omit-frame-pointer -march=native c_string.c main.c -o $(OUT_DIR)/clang_linux_test

gcc_win: | $(OUT_DIR)
	gcc -std=c99 -Wall -Wextra -g -fno-omit-frame-pointer -march=native c_string.c main.c -o $(OUT_DIR)/gcc_win_test.exe

gcc_linux: | $(OUT_DIR)
	gcc -std=c99 -Wall -Wextra -pedantic -g -fstack-protector-all -O2 -D_FORTIFY_SOURCE=2 -fno-omit-frame-pointer -march=native c_string.c main.c -o $(OUT_DIR)/gcc_linux_test


test: lib
	cd tests && $(CEEDLING) test:all

test-one: lib
ifeq ($(strip $(TEST)),)
	$(error TEST variable not set. Usage: make test-one TEST=test_name)
endif
	cd tests && $(CEEDLING) test:$(TEST)

clean:
	rm -rf $(OUT_DIR) tests/build

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
