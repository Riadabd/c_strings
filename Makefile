OUT_DIR := out
CEEDLING ?= /opt/homebrew/lib/ruby/gems/3.4.0/bin/ceedling
LIB_OBJ := $(OUT_DIR)/c_string.o
LIB := $(OUT_DIR)/libc_strings.a

.PHONY: lib test clean

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(LIB_OBJ): c_string.c c_string.h | $(OUT_DIR)
	clang -std=c99 -Wall -Wextra -pedantic -O2 -c c_string.c -o $(LIB_OBJ)

$(LIB): $(LIB_OBJ)
	ar rcs $(LIB) $(LIB_OBJ)

lib: $(LIB)

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

clean:
	rm -rf $(OUT_DIR) tests/build
