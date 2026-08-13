###############################################################################
# BeagleBone Black - Character Driver Makefile
###############################################################################

obj-m += char_driver.o

KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)


###############################################################################
# Build Kernel Module
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
# Install
###############################################################################

.PHONY: install

install: all
	sudo insmod char_driver.ko


###############################################################################
# Remove
###############################################################################

.PHONY: remove

remove:
	sudo rmmod char_driver


###############################################################################
# Check Driver
###############################################################################

.PHONY: status

status:
	lsmod | grep char_driver || true
	ls -l /dev/bbb_char 2>/dev/null || true


###############################################################################
# Kernel Logs
###############################################################################

.PHONY: logs

logs:
	dmesg | tail -30


###############################################################################
# Help
###############################################################################

.PHONY: help

help:
	@echo "BeagleBone Black Character Driver"
	@echo ""
	@echo "Build:"
	@echo "  make"
	@echo ""
	@echo "Load:"
	@echo "  sudo insmod char_driver.ko"
	@echo ""
	@echo "Remove:"
	@echo "  sudo rmmod char_driver"
	@echo ""
	@echo "Status:"
	@echo "  make status"
	@echo ""
	@echo "Logs:"
	@echo "  make logs"
	@echo ""
	@echo "Clean:"
	@echo "  make clean"
