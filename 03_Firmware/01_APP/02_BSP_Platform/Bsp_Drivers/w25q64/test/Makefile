# Host-side build for the W25Q64 driver mock test suite.
# Run from this directory:
#     make            # build ./test_w25q64_mock
#     make run        # build and run
#     make verbose    # build with DEBUG_OUT routed to stderr
#     make clean

CC      ?= gcc
CFLAGS  ?= -std=c99 -O0 -g -Wall -Wextra -Wno-unused-parameter
TARGET  := test_w25q64_mock

DRIVER_DIR := ../driver
INCLUDES   := -I./mock -I$(DRIVER_DIR)/inc

SOURCES := \
    test_w25q64_mock.c \
    $(DRIVER_DIR)/src/bsp_w25q64_driver.c

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

run: $(TARGET)
	./$(TARGET)

verbose:
	$(CC) $(CFLAGS) -DW25Q64_TEST_VERBOSE $(INCLUDES) -o $(TARGET) $(SOURCES)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe

.PHONY: run verbose clean
