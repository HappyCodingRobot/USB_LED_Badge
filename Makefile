CC=gcc
CFLAGS=-Os
LIBS=-lz -lpng -lhidapi-hidraw -lusb
OBJS=readpng.o  main.o badge.o gfx/font.o gfx/lucida_font.o gfx/courier_font.o

all: badge

badge: $(OBJS)
	gcc   $(OBJS) $(LIBS) -o badge



.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<



clean:
	rm *.o gfx/*.o badge
