# Bibliotecas SOP

CC = @gcc
CFLAGS = -O3 -Wall -Wno-sizeof-pointer-memaccess -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-result

SRC_DIR = ./src
BLD_DIR = ./build
BIN_DIR = ./bin
DOC_DIR = ./docs

COMMON	= $(SRC_DIR)/common.h $(SRC_DIR)/data.h

all: pre main

pre:
	@mkdir -p $(BIN_DIR) $(BLD_DIR) $(DOC_DIR)
	@cp ./archivo_prueba/* $(BIN_DIR)*
	@echo -e "Los ejecutables se encuentran en la carpeta '$(BIN_DIR)'"

main: $(BIN_DIR)/server $(BIN_DIR)/client

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
	@rm -rf $(BLD_DIR)/ $(BIN_DIR)/ $(DOC_DIR)
	@echo -e "Se eliminaron los ejecutables"

.PHONY: docs
docs:
	@doxygen ./doxyconf
	@echo -e "Se creó documentacion doxygen"