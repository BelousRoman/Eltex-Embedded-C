SERIAL_SERV_BIN := serial_server
CLASSIC_SERV_BIN := classic_server
CLIENT_BIN := client
MULTIPROTO_SERV_BIN := multiproto_server
MULTIPROTO_CLIENT_BIN := multiproto_client
HEADERS := hdr/*.h
SOURCES := src/*.c
CC := gcc -Wall
CFLAGS := -g
OFLAGS := -c
LIBS := -pthread
DIR_NAME := build

all: $(SERIAL_SERV_BIN) $(CLASSIC_SERV_BIN) $(CLIENT_BIN) $(MULTIPROTO_SERV_BIN) $(MULTIPROTO_CLIENT_BIN)
	mkdir -p ./build
	mv -f $(SERIAL_SERV_BIN) $(CLASSIC_SERV_BIN) $(CLIENT_BIN) $(MULTIPROTO_SERV_BIN) $(MULTIPROTO_CLIENT_BIN) ./build

dir:
	mkdir -p ./$(DIR_NAME)

$(SERIAL_SERV_BIN): serial_server.c src/serial_server.c
	$(CC) -o $(SERIAL_SERV_BIN) serial_server.c src/serial_server.c $(LIBS) $(CFLAGS)
$(SERIAL_SERV_BIN): $(HEADERS)

$(CLASSIC_SERV_BIN): classic_server.c src/classic_server.c
	$(CC) -o $(CLASSIC_SERV_BIN) classic_server.c src/classic_server.c $(LIBS) $(CFLAGS)
$(CLASSIC_SERV_BIN): $(HEADERS)

$(CLIENT_BIN): client.c src/client.c
	$(CC) -o $(CLIENT_BIN) client.c src/client.c $(LIBS) $(CFLAGS)
$(CLIENT_BIN): $(HEADERS)

$(MULTIPROTO_SERV_BIN): multiproto_server.c src/multiproto_server.c
	$(CC) -o $(MULTIPROTO_SERV_BIN) multiproto_server.c src/multiproto_server.c $(LIBS) $(CFLAGS)
$(MULTIPROTO_SERV_BIN): $(HEADERS)

$(MULTIPROTO_CLIENT_BIN): multiproto_client.c src/multiproto_client.c
	$(CC) -o $(MULTIPROTO_CLIENT_BIN) multiproto_client.c src/multiproto_client.c $(LIBS) $(CFLAGS)
$(MULTIPROTO_CLIENT_BIN): $(HEADERS)

clean:
	rm -rf ./build *.o

rebuild: clean all
