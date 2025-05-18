CC=clang
CPPFLAGS=-Wall -Werror
CFLAGS=-g
NAME=main

all: main.o
	$(CC) $(CPPFLAGS) $(CFLAGS) *.o -o $(NAME)

clean:
	rm *.o $(NAME)
