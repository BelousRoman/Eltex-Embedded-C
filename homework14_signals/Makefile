BIN := hw13_signals
HEADERS := hdr/*.h
SOURCES := src/*.c
OBJECTS := functions.o
CC := gcc -Wall
CFLAGS := -g
OFLAGS := -c

all: $(BIN)

$(BIN): main.c src/functions.c
	$(CC) -o $(BIN) main.c src/functions.c $(CFLAGS)

$(OBJECTS): $(HEADERS)

# functions.o: src/functions.c
# 	$(CC) $(OFLAGS) functions.o src/functions.c $(CFLAGS) 

clean:
	rm -f $(BIN) *.o

rebuild: clean all
