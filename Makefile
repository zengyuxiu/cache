CC = gcc
CFLAGS = -Wall -Werror -O0 -ggdb

a.out: main.c cpu.c cache.c mem.c
	$(CC) $(CFLAGS) -o $@ $^ 

clean:
	-rm a.out

.PHONY: clean
