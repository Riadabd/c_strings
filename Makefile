clang_win:
	clang -Weverything -g -fsanitize=undefined -fno-omit-frame-pointer -march=native c_string.c main.c -o test.exe

clang_linux:
	clang -Weverything -g -fsanitize=address,memory,undefined -fno-omit-frame-pointer -march=native c_string.c main.c -o test

gcc_win:
	gcc -Wall -Wextra -g -fno-omit-frame-pointer -march=native c_string.c main.c -o test.exe

gcc_linux:
	gcc -Wall -Wextra -g -fno-omit-frame-pointer -march=native c_string.c main.c -o test
