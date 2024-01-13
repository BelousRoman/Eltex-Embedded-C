SSERV_BIN := serial_server
CSERV_BIN := classic_server
CLIENT_BIN := client
HEADERS := hdr/*.h
SOURCES := src/*.c
CC := gcc -Wall
CFLAGS := -g
OFLAGS := -c

all: $(SSERV_BIN) $(CSERV_BIN) $(CLIENT_BIN)

$(SSERV_BIN): serial_server.c src/serial_server.c
	$(CC) -o $(SSERV_BIN) serial_server.c src/serial_server.c $(CFLAGS)
$(SSERV_BIN): $(HEADERS)

$(CSERV_BIN): classic_server.c src/classic_server.c
	$(CC) -o $(CSERV_BIN) classic_server.c src/classic_server.c $(CFLAGS)
$(CSERV_BIN): $(HEADERS)

$(CLIENT_BIN): client.c src/client.c
	$(CC) -o $(CLIENT_BIN) client.c src/client.c $(CFLAGS)
$(CLIENT_BIN): $(HEADERS)

clean:
	rm -f $(SSERV_BIN) $(CSERV_BIN) $(CLIENT_BIN) *.o

rebuild: clean all
