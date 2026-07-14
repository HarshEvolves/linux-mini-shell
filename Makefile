CC ?= gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99 -O2 -D_POSIX_C_SOURCE=200809L -Iinclude
SRC_DIR = src
BUILD_DIR = build
TARGET  = $(BUILD_DIR)/minishell

PREFIX ?= /usr/local
BINDIR = $(DESTDIR)$(PREFIX)/bin

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

install: all
	@echo "Installing minishell to $(BINDIR)..."
	mkdir -p $(BINDIR)
	cp $(TARGET) $(BINDIR)/minishell
	chmod 755 $(BINDIR)/minishell
	@echo "Installation successful! Run 'minishell' to start."

uninstall:
	@echo "Removing minishell from $(BINDIR)..."
	rm -f $(BINDIR)/minishell
	@echo "Uninstallation successful!"

test: all
	@echo "Running test suite..."
	@if [ -x tests/run_tests.sh ]; then \
		./tests/run_tests.sh; \
	else \
		echo "No tests implemented yet."; \
	fi

clean:
	rm -rf $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run install uninstall test
