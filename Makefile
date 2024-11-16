all: words
words: words.c
	gcc -Wall -Werror -fsanitize=address,undefined -g words.c -o words
clear:
	rm -rf words *~
