BIN_VFS := hw10_vfs
BIN_IDE := hw10_texteditor
HEADERS := hdr/*.h
SOURCES := src/*.c
OBJECTS := functions.o
CC := gcc -Wall
CFLAGS := -g
OFLAGS := -c
LIBS := -lcurses

all: $(BIN_VFS) $(BIN_IDE)

$(BIN_VFS): main.c src/functions.c
	$(CC) -o $(BIN_VFS) main.c src/functions.c $(CFLAGS) $(LIBS)
$(BIN_VFS): $(HEADERS)

$(BIN_IDE): texteditor.c src/functions.c
	$(CC) -o $(BIN_IDE) texteditor.c src/functions.c $(CFLAGS) $(LIBS)
$(BIN_IDE): $(HEADERS)

clean:
	rm -f $(BIN_VFS) $(BIN_IDE) *.o

rebuild: clean all
