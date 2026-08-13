###############################################################################
# BeagleBone Black - Button Input Driver Makefile
#
# Driver:
#     button_input.c
#
# Output:
#     button_input.ko
###############################################################################

obj-m += button_input.o


###############################################################################
# Kernel Source
###############################################################################

KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)


###############################################################################
# Build
###############################################################################

.PHONY: all

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules


###############################################################################
# Clean
###############################################################################

.PHONY: clean

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean


###############################################################################
# Load
###############################################################################

.PHONY: load

load: all
	sudo insmod button_input.ko


###############################################################################
# Unload
###############################################################################

.PHONY: unload

unload:
	sudo rmmod button_input


###############################################################################
# Status
###############################################################################

.PHONY: status

status:
	@echo "Button driver:"
	@lsmod | grep button_input || true
	@echo ""
	@echo "Input devices:"
	@ls -l /dev/input/ 2>/dev/null || true


###############################################################################
# Input Devices
###############################################################################

.PHONY: info

info:
	@echo "Input devices:"
	@cat /proc/bus/input/devices | grep -A6 -B2 -i "BBB GPIO Button" || true


###############################################################################
# Kernel Logs
###############################################################################

.PHONY: logs

logs:
	dmesg | grep -i button


###############################################################################
# Test
###############################################################################

.PHONY: test

test:
	@echo "Checking input subsystem..."
	@if [ -d /dev/input ]; then \
		echo "Input subsystem available."; \
	else \
		echo "Input subsystem not available."; \
	fi


###############################################################################
# Help
###############################################################################

.PHONY: help

help:
	@echo "BeagleBone Black Button Input Driver"
	@echo ""
	@echo "  make          - Build driver"
	@echo "  make clean    - Clean build"
	@echo "  make load     - Load driver"
	@echo "  make unload   - Unload driver"
	@echo "  make status   - Show driver status"
	@echo "  make info     - Show input device"
	@echo "  make test     - Test input subsystem"
	@echo "  make logs     - Show kernel logs"
