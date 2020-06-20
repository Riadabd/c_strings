clang_win:
	clang -Wall -g -fsanitize=address,undefined -fno-omit-frame-pointer c_string.c main.c -o test.exe

clang_linux:
	clang -Wall -g -fsanitize=address,memory,undefined -fno-omit-frame-pointer c_string.c main.c -o test

gcc_win:
	gcc -Wall -Wextra -g -fno-omit-frame-pointer c_string.c main.c -o test.exe

gcc_linux:
	gcc -Wall -Wextra -g -fno-omit-frame-pointer c_string.c main.c -o test