###############################################################################
# BeagleBone Black - UART Driver Makefile
#
# Driver:
#     uart_driver.c
#
# Output:
#     uart_driver.ko
###############################################################################

obj-m += uart_driver.o


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
	sudo insmod uart_driver.ko


###############################################################################
# Unload
###############################################################################

.PHONY: unload

unload:
	sudo rmmod uart_driver


###############################################################################
# Status
###############################################################################

.PHONY: status

status:
	@echo "UART driver:"
	@lsmod | grep uart_driver || true
	@echo ""
	@echo "UART device:"
	@ls -l /dev/bbb_uart 2>/dev/null || true


###############################################################################
# Kernel Logs
###############################################################################

.PHONY: logs

logs:
	dmesg | tail -50


###############################################################################
# Test
###############################################################################

.PHONY: test

test:
	@if [ -e /dev/bbb_uart ]; then \
		echo "UART device exists: /dev/bbb_uart"; \
	else \
		echo "ERROR: /dev/bbb_uart not found"; \
	fi


###############################################################################
# Help
###############################################################################

.PHONY: help

help:
	@echo "BeagleBone Black UART Driver"
	@echo ""
	@echo "  make          - Build driver"
	@echo "  make clean    - Clean build"
	@echo "  make load     - Load driver"
	@echo "  make unload   - Unload driver"
	@echo "  make status   - Show driver status"
	@echo "  make logs     - Show kernel logs"
	@echo "  make test     - Check device node"
