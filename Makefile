clang_win:
	clang -Weverything -g -fsanitize=undefined -fno-omit-frame-pointer -march=native c_string.c main.c -o clang_win_test.exe

clang_linux:
	clang -Weverything -g -fsanitize=address,undefined -fno-omit-frame-pointer -march=native c_string.c main.c -o clang_linux_test

gcc_win:
	gcc -Wall -Wextra -g -fno-omit-frame-pointer -march=native c_string.c main.c -o gcc_win_test.exe

gcc_linux:
	gcc -Wall -Wextra -pedantic -g -fstack-protector-all -O2 -D_FORTIFY_SOURCE=2 -fno-omit-frame-pointer -march=native c_string.c main.c -o gcc_linux_test
