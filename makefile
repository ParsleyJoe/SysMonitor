CFLAGS = -g -Wall -Wextra -Wshadow -Wpedantic -Wconversion
SANFLAGS = -fsanitize=undefined -fsanitize=address
TARGET = monitor

monitor: main.o proc.o ui.o
	cc $(SANFLAGS) -o monitor main.o proc.o ui.o -lncurses -lpthread

main.o: src/main.c
	cc $(CFLAGS) $(SANFLAGS) -c src/main.c -o main.o

proc.o: src/proc.c
	cc $(CFLAGS) $(SANFLAGS) -c src/proc.c -o proc.o

ui.o: src/ui.c
	cc $(CLFAGS) $(SANFLAGS) -c src/ui.c -o ui.o

clean:
	rm $(TARGET) main.o proc.o
