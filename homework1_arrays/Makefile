BIN := hw1_arrays
HEADERS := include/*.h
SOURCES := src/*.c
OBJECTS := functions.o
CC := gcc -Wall
CFLAGS := -g
OFLAGS := -c
LIBS := -lm

all: main.c $(OBJECTS)
	$(CC) main.c $(OBJECTS) $(LIBS) -o $(BIN)

functions.o: $(HEADERS) $(SOURCES)
	$(CC) $(SOURCES) $(OFLAGS) functions.o

clean:
	rm -f $(BIN) *.o

rebuild: clean all