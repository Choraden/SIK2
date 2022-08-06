SRC=cmd.c err.c net.c read_line.c

CC = gcc

CFLAGS=-std=c99 -Wall -Wunused

gui: gui.c $(SRC) gui.h 
	$(CC) $(CFLAGS) gui.c $(SRC) -o gui `pkg-config gtk+-2.0 --cflags --libs`

clean: 
	rm -f *.o gui
