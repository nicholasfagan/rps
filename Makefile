# Rock Paper Scissors Compilation / Excecution
# Feb 25th 2019
# Nick Fagan
# B00767175 

PORT=1337
HOST=127.0.0.1

CC= /usr/bin/gcc
CCFLAGS= -std=c99 -Wall -Ofast

PREFIX= /usr/bin

SRC= ./src
BIN= ./bin

exec= $(BIN)/server $(BIN)/client

all: build

$(BIN)/%: $(SRC)/%.c $(SRC)/%.h $(SRC)/rps.h
	@mkdir -p $(BIN)
	$(CC) $(CCFLAGS) $< -o $@

server: $(BIN)/server
	$< $(PORT)

client: $(BIN)/client
	$< $(HOST) $(PORT)

clean:
	rm -rf $(exec) $(BIN)

build: $(exec)

test: $(exec)
	$(BIN)/server $(PORT) >/dev/null 2>&1 &
	$(BIN)/client $(HOST) $(PORT)
