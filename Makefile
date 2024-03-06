CC  := gcc

CFLAGS  :=

BIN_DIR := bin

SRC_DIR := src
INC_DIR := include

TARGET  := $(BIN_DIR)/libpnet.a
SOURCES := $(wildcard $(SRC_DIR)/**.c)
OBJECTS := $(SOURCES:.c=.o)

.PHONY: all

all: $(TARGET) clean

$(BIN_DIR):
	@mkdir -p $@

clean:
	@rm -rf $(OBJECTS)

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	@ar rcs $@ $^

$(OBJECTS): $(SOURCES)
	@$(CC) -c $(CFLAGS) -o $@ $< -I$(INC_DIR)