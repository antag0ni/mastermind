# MasterMind Makefile

CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99
LDFLAGS = -lncurses
TARGET = mastermind
SRC = mastermind.c
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

.PHONY: all clean run install uninstall

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

install: $(TARGET)
	@echo "Installing $(TARGET) to $(BINDIR)..."
	@install -d $(BINDIR)
	@install -m 755 $(TARGET) $(BINDIR)/
	@echo "Installation complete! You can now run 'mastermind' from anywhere."

uninstall:
	@echo "Removing $(TARGET) from $(BINDIR)..."
	@rm -f $(BINDIR)/$(TARGET)
	@echo "Uninstallation complete."
