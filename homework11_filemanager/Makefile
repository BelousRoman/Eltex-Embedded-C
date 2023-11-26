BIN := hw11_fm
HEADERS := hdr/*.h
SOURCES := src/*.c
OBJECTS := functions.o
CC := gcc -Wall
CFLAGS := -g
OFLAGS := -c
LIBS := -lcurses

vpath %.h hdr/
vpath %.c src/

all: $(BIN)

$(BIN): main.c $(OBJECTS)
	$(CC) -o $(BIN) main.c $(OBJECTS) $(CFLAGS) $(LIBS)

$(OBJECTS): $(HEADERS)

functions.o: src/functions.c
	$(CC) $(OFLAGS) functions.o src/functions.c $(CFLAGS)

clean:
	rm -f $(BIN) *.o

rebuild: clean all
