SERVER_BIN := server
CLIENT_BIN := client
HEADERS := hdr/*.h
SOURCES := src/*.c
OBJECTS := client.o server.o
CC := gcc -Wall
CFLAGS := -g
OFLAGS := -c
LIBS := -pthread
DIR_NAME := build

all: dir $(SERVER_BIN) $(CLIENT_BIN)

dir:
	mkdir -p $(DIR_NAME)

%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $(DIR_NAME)/$@
%.o: $(HEADERS)

$(SERVER_BIN): server.c server.o
	$(CC) -o $(DIR_NAME)/$(SERVER_BIN) server.c $(DIR_NAME)/server.o $(LIBS) $(CFLAGS)

$(CLIENT_BIN): client.c client.o
	$(CC) -o $(DIR_NAME)/$(CLIENT_BIN) client.c $(DIR_NAME)/client.o $(LIBS) $(CFLAGS)

clean:
	rm -rf ./$(DIR_NAME) *.o

rebuild: clean all
