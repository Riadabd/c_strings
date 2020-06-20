clang_win:
	clang -Wall -g -fsanitize=undefined -fno-omit-frame-pointer c_string.c main.c -o test.exe

gcc_win:
	gcc -Wall -Wextra -g -fno-omit-frame-pointer c_string.c main.c -o test.exe