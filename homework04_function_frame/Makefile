PROG_BIN := hw4_function_frame
UTIL_BIN := bypass_util
CC := gcc -Wall
BIN_SRC := test.c
BIN_OBJ := test.o
UTIL_SRC := main.c
UTIL_OBJ := main.o
CFLAGS := -g 
OFLAGS := -c 
ADDRFLAGS := -fno-stack-protector -no-pie
DIR := build

all: dir fixed_addressing $(DIR)/$(UTIL_BIN)

dir:
	mkdir -p $(DIR)

$(DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

fixed_addressing: $(BIN_SRC)
	$(CC) -o $(DIR)/$(PROG_BIN) $(BIN_SRC) $(CFLAGS) $(ADDRFLAGS)

normal_addressing: $(BIN_SRC)
	$(CC) -o $(DIR)/$(PROG_BIN) $(BIN_SRC) $(CFLAGS)

$(DIR)/$(UTIL_BIN): $(DIR)/$(UTIL_OBJ)
	$(CC) -o $(DIR)/$(UTIL_BIN) $(DIR)/$(UTIL_OBJ) $(CFLAGS)

clean:
	rm -rf ./$(DIR) *.o $(PROG_BIN) $(UTIL_BIN)

rebuild: clean all
