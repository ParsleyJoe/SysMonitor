CFLAGS = -g -Wall -Wextra -Wshadow -Wpedantic

monitor: main.o
	cc -o monitor main.o

main.o: main.c
	cc $(CFLAGS) -c main.c -o main.o

clean:
	rm main.o monitor
