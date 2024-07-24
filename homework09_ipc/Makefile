BIN := hw9_ipc
HEADERS := hdr/*.h
SOURCES := src/*.c
BUILD_DIR := $(PWD)/build
KBUF_DIR := $(PWD)/kbuffer
OBJECTS := functions.o
CC := gcc -Wall
CFLAGS := -g
OFLAGS := -c

all: $(BUILD_DIR) $(KBUF_DIR) $(BIN)

$(BUILD_DIR):
	mkdir -p "$@"

$(KBUF_DIR):
	mkdir -p "$@"

$(BIN): main.c functions.o first_proc second_proc
	$(CC) -o $(BIN) main.c functions.o $(CFLAGS)
$(OBJECTS): $(HEADERS)

functions.o: src/functions.c hdr/functions.h
	$(CC) $(OFLAGS) functions.o src/functions.c $(CFLAGS)

first_proc: first_proc.c hdr/functions.h
	$(CC) -o $(BUILD_DIR)/first_proc first_proc.c functions.o $(CFLAGS)

second_proc: second_proc.c hdr/functions.h
	$(CC) -o $(BUILD_DIR)/second_proc second_proc.c functions.o $(CFLAGS)

clean:
	rm -f $(BIN) $(BUILD_DIR)/* $(KBUF_DIR)/* *.o && rmdir $(BUILD_DIR) $(KBUF_DIR)

rebuild: clean all
