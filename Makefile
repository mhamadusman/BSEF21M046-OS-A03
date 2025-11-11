# Makefile for FCIT Shell project (matches src/ and include/ layout)

CC = gcc
CFLAGS = -Wall -g -Iinclude

SRCDIR = src
BINDIR = bin

SRC = $(SRCDIR)/main.c $(SRCDIR)/shell.c $(SRCDIR)/execute.c
OBJ = $(SRC:.c=.o)
TARGET = $(BINDIR)/myshell

.PHONY: all clean run shellsy

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SRCDIR)/*.o $(TARGET)

run: all
	$(TARGET)

# convenience: create ./shell symlink to bin/myshell
shellsy: all
	ln -sf $(TARGET) ./shell
	@echo "Created ./shell -> $(TARGET). You can now run ./shell"
