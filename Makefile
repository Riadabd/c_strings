clang_win:
	mkdir -p out
	clang -std=c99 -Weverything -g -fsanitize=undefined -fno-omit-frame-pointer -march=native c_string.c main.c -o out/clang_win_test.exe

clang_linux:
	mkdir -p out
	clang -std=c99 -Weverything -g -fsanitize=address,undefined -fno-omit-frame-pointer -march=native c_string.c main.c -o out/clang_linux_test

gcc_win:
	mkdir -p out
	gcc -std=c99 -Wall -Wextra -g -fno-omit-frame-pointer -march=native c_string.c main.c -o out/gcc_win_test.exe

gcc_linux:
	mkdir -p out
	gcc -std=c99 -Wall -Wextra -pedantic -g -fstack-protector-all -O2 -D_FORTIFY_SOURCE=2 -fno-omit-frame-pointer -march=native c_string.c main.c -o out/gcc_linux_test
