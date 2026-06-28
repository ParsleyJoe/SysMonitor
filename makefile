CFLAGS = -g -Wall -Wextra -Wshadow -Wpedantic -Wconversion
SANFLAGS = -fsanitize=undefined
TARGET = monitor

monitor: main.o
	cc $(SANFLAGS) -o monitor main.o

main.o: main.c
	cc $(CFLAGS) $(SANFLAGS) -c main.c -o main.o

clean:
	rm $(TARGET) main.o
