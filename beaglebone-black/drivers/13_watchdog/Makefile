###############################################################################
# BeagleBone Black - Watchdog Demo Makefile
#
# Output:
#     watchdog_demo.ko
###############################################################################

obj-m += watchdog_demo.o


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
	sudo insmod watchdog_demo.ko


###############################################################################
# Unload
###############################################################################

.PHONY: unload

unload:
	sudo rmmod watchdog_demo


###############################################################################
# Status
###############################################################################

.PHONY: status

status:
	@echo "Watchdog driver:"
	@lsmod | grep watchdog_demo || true
	@echo ""
	@echo "Watchdog device:"
	@ls -l /dev/watchdog* 2>/dev/null || true


###############################################################################
# Logs
###############################################################################

.PHONY: logs

logs:
	dmesg | grep -i watchdog


###############################################################################
# Test
###############################################################################

.PHONY: test

test:
	@echo "Watchdog device:"
	@ls -l /dev/watchdog* 2>/dev/null || true
	@echo ""
	@echo "Watchdog information:"
	@cat /sys/class/watchdog/watchdog0/identity 2>/dev/null || true


###############################################################################
# Help
###############################################################################

.PHONY: help

help:
	@echo "BeagleBone Black Watchdog Demo"
	@echo ""
	@echo "  make          - Build driver"
	@echo "  make clean    - Clean build"
	@echo "  make load     - Load driver"
	@echo "  make unload   - Unload driver"
	@echo "  make status   - Show driver status"
	@echo "  make logs     - Show kernel logs"
	@echo "  make test     - Test watchdog"
