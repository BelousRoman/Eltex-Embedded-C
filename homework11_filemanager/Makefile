BIN := hw11_fm
HEADERS := hdr/*.h
SOURCES := src/*.c
CC := gcc -Wall
CFLAGS := -g
OFLAGS := -c
LIBS := -lcurses

all: $(BIN)

$(BIN): main.c src/functions.c
	$(CC) -o $(BIN) main.c src/functions.c $(CFLAGS) $(LIBS)
$(BIN): $(HEADERS)

clean:
	rm -f $(BIN) *.o

rebuild: clean all
