BIN := hw14_signals
HEADERS := hdr/*.h
SOURCES := src/*.c
CC := gcc -Wall
CFLAGS := -g
OFLAGS := -c

all: $(BIN)

$(BIN): main.c src/functions.c
	$(CC) -o $(BIN) main.c src/functions.c $(CFLAGS)
$(BIN): $(HEADERS)

clean:
	rm -f $(BIN) *.o

rebuild: clean all
