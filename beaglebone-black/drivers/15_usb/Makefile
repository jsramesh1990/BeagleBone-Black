###############################################################################
# BeagleBone Black - USB Driver Makefile
#
# Output:
#     usb_driver.ko
###############################################################################

obj-m += usb_driver.o


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
	sudo insmod usb_driver.ko


###############################################################################
# Unload
###############################################################################

.PHONY: unload

unload:
	sudo rmmod usb_driver


###############################################################################
# Status
###############################################################################

.PHONY: status

status:
	@echo "USB driver:"
	@lsmod | grep usb_driver || true

	@echo ""
	@echo "USB devices:"
	@lsusb 2>/dev/null || true


###############################################################################
# Logs
###############################################################################

.PHONY: logs

logs:
	dmesg | grep -i "usb"


###############################################################################
# USB Information
###############################################################################

.PHONY: info

info:
	@echo "USB devices detected:"
	@lsusb 2>/dev/null || true


###############################################################################
# Test
###############################################################################

.PHONY: test

test:
	@echo "USB driver test"
	@echo ""
	@echo "1. Load the driver:"
	@echo "   sudo insmod usb_driver.ko"
	@echo ""
	@echo "2. Connect the matching USB device."
	@echo ""
	@echo "3. Check:"
	@echo "   dmesg | tail -30"
	@echo ""
	@echo "4. Check USB devices:"
	@echo "   lsusb"


###############################################################################
# Help
###############################################################################

.PHONY: help

help:
	@echo "BeagleBone Black USB Driver"
	@echo ""
	@echo "  make          - Build driver"
	@echo "  make clean    - Clean build"
	@echo "  make load     - Load driver"
	@echo "  make unload   - Unload driver"
	@echo "  make status   - Show driver and USB status"
	@echo "  make info     - Show USB devices"
	@echo "  make logs     - Show USB kernel logs"
	@echo "  make test     - Show USB test procedure"
