# Project configuration
CC ?= cc
TARGET ?= flappy_bird
SRC_DIR := src
INCLUDE_DIR := include
BUILD_DIR := build

# Compiler and linker configuration
CSTD ?= c11
WARN_FLAGS ?= -Wall -Wextra -Werror -Wpedantic -Wno-unused-parameter
CPPFLAGS ?= -I$(INCLUDE_DIR) -MMD -MP
CFLAGS ?= -std=$(CSTD) $(WARN_FLAGS)
LDFLAGS ?=
LDLIBS ?= -lcurses

SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
DEPS := $(OBJECTS:.o=.d)
HEADERS := $(wildcard $(INCLUDE_DIR)/flappybird/*.h)

.PHONY: all clean run debug release format lint help

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

debug: CFLAGS += -O0 -g3
debug: clean all

release: CFLAGS += -O2 -DNDEBUG
release: clean all

format:
	clang-format -i $(SOURCES) $(HEADERS)

lint:
	@if command -v cppcheck >/dev/null 2>&1; then \
		cppcheck --enable=warning,performance,portability \
			--std=$(CSTD) --error-exitcode=1 \
			-I$(INCLUDE_DIR) $(SRC_DIR) $(INCLUDE_DIR); \
	else \
		echo "cppcheck not found; skipping static analysis."; \
	fi

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

help:
	@echo "Targets:"
	@echo "  all      - build the game binary"
	@echo "  run      - build and run the game"
	@echo "  debug    - clean and build with debug flags"
	@echo "  release  - clean and build optimized release binary"
	@echo "  format   - format C sources/headers with clang-format"
	@echo "  lint     - run cppcheck static analysis"
	@echo "  clean    - remove build artifacts"

-include $(DEPS)
