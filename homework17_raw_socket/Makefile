SERV_BIN := server
CLIENT_BIN := client
HEADERS := hdr/*.h
SOURCES := src/*.c
OBJECTS := client.o server.o
CC := gcc -Wall
CFLAGS := -g
OFLAGS := -c
LIBS := -pthread
DIR_NAME := build

all: create_dir $(SERV_BIN) $(CLIENT_BIN)
	# mkdir -p ./build
	# mv -f $(SERV_BIN) $(CLIENT_BIN) ./build

create_dir:
	mkdir -p ./$(DIR_NAME)

$(SERV_BIN): server.c src/server.c
	$(CC) -o $(DIR_NAME)/$(SERV_BIN) server.c src/server.c $(LIBS) $(CFLAGS)
$(SERV_BIN): $(HEADERS)

$(CLIENT_BIN): client.c src/client.c
	$(CC) -o $(DIR_NAME)/$(CLIENT_BIN) client.c src/client.c $(LIBS) $(CFLAGS)
$(CLIENT_BIN): $(HEADERS)

# $(SERV_BIN): server.c $(OBJECTS)
# 	$(CC) -o $(DIR_NAME)/$(SERV_BIN) server.c $(DIR_NAME)/$(OBJECTS) $(LIBS) $(CFLAGS)
# $(SERV_BIN): $(HEADERS)

# $(CLIENT_BIN): client.c $(OBJECTS)
# 	$(CC) -o $(DIR_NAME)/$(CLIENT_BIN) client.c $(DIR_NAME)/$(OBJECTS) $(LIBS) $(CFLAGS)
# $(CLIENT_BIN): $(HEADERS)

client.o: $(HEADERS) $(SOURCES)
	$(CC) functions.o $(OFLAGS) $(SOURCES) 

test: $(OBJECTS)

$(DIR_NAME)/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(HEADERS) -c $< -o $@

clean:
	rm -rf ./$(DIR_NAME) *.o

rebuild: clean all
