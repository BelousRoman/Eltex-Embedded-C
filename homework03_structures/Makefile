BIN := hw3_structures
HEADERS := hdr/*.h
SOURCES := src/*.c
OBJECTS := functions.o
CC := gcc -Wall
CFLAGS := -g
OFLAGS := -c
LIBS := -pthread
DIR_NAME := build

all: dir $(BIN) $(CLIENT_BIN)

dir:
	mkdir -p $(DIR_NAME)

%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $(DIR_NAME)/$@
%.o: $(HEADERS)

$(BIN): main.c functions.o
	$(CC) -o $(DIR_NAME)/$(BIN) main.c $(DIR_NAME)/functions.o $(LIBS) $(CFLAGS)

clean:
	rm -rf ./$(DIR_NAME)

rebuild: clean all
