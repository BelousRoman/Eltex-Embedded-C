BIN := hw7_threads
HEADERS := hdr/*.h
SOURCES := src/*.c
OBJECTS := functions.o
CC := gcc -Wall
CFLAGS := -g
OFLAGS := -c
LIBS := -lm

vpath %.h hdr/
vpath %.c src/

all: $(BIN)

$(BIN): main.c $(OBJECTS)
	$(CC) -o $(BIN) main.c $(OBJECTS) $(CFLAGS)
$(OBJECTS): $(HEADERS)

functions.o: src/functions.c
	$(CC) $(OFLAGS) functions.o src/functions.c $(CFLAGS)

# functions.o: functions.c functions.h
# 	$(CC) $(OFLAGS) -o functions.o functions.c $(CFLAGS)


clean:
	rm -f $(BIN) *.o

rebuild: clean all
