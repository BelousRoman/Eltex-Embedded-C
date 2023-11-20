BIN_VFS := hw10_vfs
BIN_IDE := hw10_texteditor
HEADERS := hdr/*.h
SOURCES := src/*.c
OBJECTS := functions.o
CC := gcc -Wall
CFLAGS := -g
OFLAGS := -c
LIBS := -lcurses

vpath %.h hdr/
vpath %.c src/

all: $(BIN_VFS) $(BIN_IDE)

$(BIN_VFS): main.c $(OBJECTS)
	$(CC) -o $(BIN_VFS) main.c $(OBJECTS) $(CFLAGS) $(LIBS)

$(BIN_IDE): texteditor.c $(OBJECTS)
	$(CC) -o $(BIN_IDE) texteditor.c $(OBJECTS) $(CFLAGS) $(LIBS)
$(OBJECTS): $(HEADERS)

functions.o: src/functions.c
	$(CC) $(OFLAGS) functions.o src/functions.c $(CFLAGS)

# functions.o: functions.c functions.h
# 	$(CC) $(OFLAGS) -o functions.o functions.c $(CFLAGS)


clean:
	rm -f $(BIN_VFS) $(BIN_IDE) *.o

rebuild: clean all
