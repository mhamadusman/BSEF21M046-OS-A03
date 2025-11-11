# Makefile for the Mini Shell Project

CC = gcc
CFLAGS = -Wall -Iinclude
SRC = src/main.c src/shell.c src/execute.c
OBJ = $(SRC:.c=.o)
BIN_DIR = bin
TARGET = $(BIN_DIR)/myshell

# Default rule
all: $(TARGET)

# Link object files into the final binary
$(TARGET): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Compile each .c file into .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -rf $(OBJ) $(TARGET) $(BIN_DIR)

# Run the shell
run: all
	./$(TARGET)
