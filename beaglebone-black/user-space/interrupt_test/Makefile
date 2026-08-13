###############################################################################
# BeagleBone Black - Interrupt Test Makefile
#
# Target:
#     interrupt_test
#
# Source:
#     interrupt_test.c
#     interrupt_test.h
#
# Interface:
#     Linux GPIO character device / libgpiod
###############################################################################

CC      := gcc

TARGET  := interrupt_test

SRC     := interrupt_test.c

OBJ     := $(SRC:.c=.o)

CFLAGS  := -Wall \
           -Wextra \
           -Werror \
           -O2 \
           -std=c11 \
           -D_GNU_SOURCE

LDLIBS  := -lgpiod

###############################################################################
# Default Target
###############################################################################

.PHONY: all

all: $(TARGET)

###############################################################################
# Build Application
###############################################################################

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

###############################################################################
# Compile Source
###############################################################################

interrupt_test.o: interrupt_test.c interrupt_test.h
	$(CC) $(CFLAGS) -c $< -o $@

###############################################################################
# Clean
###############################################################################

.PHONY: clean

clean:
	rm -f $(TARGET) $(OBJ)

###############################################################################
# Install
###############################################################################

.PHONY: install

install:
	install -d /usr/local/bin
	install -m 0755 $(TARGET) /usr/local/bin/$(TARGET)

###############################################################################
# Uninstall
###############################################################################

.PHONY: uninstall

uninstall:
	rm -f /usr/local/bin/$(TARGET)

###############################################################################
# Rising Edge Test
###############################################################################

.PHONY: rising

rising: $(TARGET)
	sudo ./$(TARGET) /dev/gpiochip0 20 rising

###############################################################################
# Falling Edge Test
###############################################################################

.PHONY: falling

falling: $(TARGET)
	sudo ./$(TARGET) /dev/gpiochip0 20 falling

###############################################################################
# Both Edge Test
###############################################################################

.PHONY: both

both: $(TARGET)
	sudo ./$(TARGET) /dev/gpiochip0 20 both

###############################################################################
# Help
###############################################################################

.PHONY: help

help:
	@echo "BeagleBone Black GPIO Interrupt Test"
	@echo ""
	@echo "Build:"
	@echo "  make"
	@echo ""
	@echo "Interrupt tests:"
	@echo "  make rising"
	@echo "  make falling"
	@echo "  make both"
	@echo ""
	@echo "Clean:"
	@echo "  make clean"
