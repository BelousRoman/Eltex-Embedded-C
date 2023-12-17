BIN := hw13_shm
SERVER_BIN := hw13_chatroom_server
CLIENT_BIN := hw13_chatroom_client
HEADERS := hdr/*.h
SOURCES := src/*.c
CC := gcc -Wall
CFLAGS := -g
OFLAGS := -c
LIBS := -lcurses

all: $(BIN) $(SERVER_BIN) $(CLIENT_BIN)

$(BIN): main.c src/functions.c
	$(CC) -o $(BIN) main.c src/functions.c $(CFLAGS) $(LIBS)
$(BIN): $(HEADERS)

$(SERVER_BIN): server.c src/server.c
	$(CC) -o $(SERVER_BIN) server.c src/server.c $(CFLAGS) $(LIBS)
$(SERVER_BIN): $(HEADERS)

$(CLIENT_BIN): client.c src/client.c
	$(CC) -o $(CLIENT_BIN) client.c src/client.c $(CFLAGS) $(LIBS)
$(CLIENT_BIN): $(HEADERS)

clean:
	rm -f $(BIN) $(SERVER_BIN) $(CLIENT_BIN) *.o

rebuild: clean all