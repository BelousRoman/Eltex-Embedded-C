BIN := hw6_processes
HEADERS := hdr/*.h
SOURCES := src/*.c
OBJECTS := functions.o
CC := gcc -Wall
CFLAGS := -g
OFLAGS := -c
LIBS := -lm

all: main.c $(OBJECTS)
	$(CC) main.c $(OBJECTS) $(LIBS) -o $(BIN) -g

functions.o: $(SOURCES)
	$(CC) $(SOURCES) $(OFLAGS) functions.o

functions.o: $(HEADERS)

clean:
	rm -f $(BIN) *.o

rebuild: clean all
