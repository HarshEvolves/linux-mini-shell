CC ?= gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99 -O2 -D_POSIX_C_SOURCE=200809L -Iinclude
SRC_DIR = src
BUILD_DIR = build
TARGET  = minishell

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

test: all
	@echo "Running test suite..."
	@if [ -x tests/run_tests.sh ]; then \
		./tests/run_tests.sh; \
	else \
		echo "No tests implemented yet."; \
	fi

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run test
