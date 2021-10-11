# Bibliotecas SOP

CC = gcc
CFLAGS = -g -Wall -Wno-sizeof-pointer-memaccess -Wno-format-overflow -Wno-unused-but-set-variable -Wno-unused-variable

SRC_DIR = ./src
BLD_DIR = ./build
BIN_DIR = ./bin

COMMON	= $(SRC_DIR)/common.h $(SRC_DIR)/data.h

all: $(BIN_DIR)/server $(BIN_DIR)/client

# Compilación del Servidor
$(BIN_DIR)/server: $(BLD_DIR)/server.o
	$(CC) $(CFLAGS) $< -o $@

$(BLD_DIR)/server.o: $(SRC_DIR)/server.c $(SRC_DIR)/server.h $(COMMON)
	$(CC) -c $(CFLAGS) $< -o $@

# Compilaciónd del Cliente
$(BIN_DIR)/client: $(BLD_DIR)/client.o
	$(CC) $(CFLAGS) $< -o $@

$(BLD_DIR)/client.o: $(SRC_DIR)/client.c $(SRC_DIR)/client.h $(COMMON)
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(BLD_DIR)/* $(BIN_DIR)/*