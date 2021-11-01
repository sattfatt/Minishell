setup:
	gcc -std=gnu99 -g3 -Wall -o smallsh smallsh.c parser.c utils.c signals.c
clean:
	rm smallsh