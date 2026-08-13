###############################################################################
# BeagleBone Black - RTC Driver Makefile
#
# Driver:
#     rtc_driver.c
#
# Output:
#     rtc_driver.ko
###############################################################################

obj-m += rtc_driver.o


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
	sudo insmod rtc_driver.ko


###############################################################################
# Unload
###############################################################################

.PHONY: unload

unload:
	sudo rmmod rtc_driver


###############################################################################
# Status
###############################################################################

.PHONY: status

status:
	@echo "RTC driver:"
	@lsmod | grep rtc_driver || true
	@echo ""
	@echo "RTC devices:"
	@ls -l /dev/rtc* 2>/dev/null || true


###############################################################################
# RTC Information
###############################################################################

.PHONY: info

info:
	@echo "RTC devices:"
	@ls -l /sys/class/rtc/ 2>/dev/null || true


###############################################################################
# Read RTC Time
###############################################################################

.PHONY: read

read:
	@for rtc in /sys/class/rtc/rtc*; do \
		if [ -d "$$rtc" ]; then \
			echo "----------------------------------------"; \
			echo "RTC: $$rtc"; \
			cat "$$rtc/name" 2>/dev/null || true; \
		fi; \
	done


###############################################################################
# Kernel Logs
###############################################################################

.PHONY: logs

logs:
	dmesg | grep -i rtc


###############################################################################
# Test
###############################################################################

.PHONY: test

test:
	@echo "Checking RTC devices..."
	@if ls /dev/rtc* >/dev/null 2>&1; then \
		echo "RTC device detected."; \
	else \
		echo "No RTC device detected."; \
	fi


###############################################################################
# Help
###############################################################################

.PHONY: help

help:
	@echo "BeagleBone Black RTC Driver"
	@echo ""
	@echo "  make          - Build driver"
	@echo "  make clean    - Clean build"
	@echo "  make load     - Load driver"
	@echo "  make unload   - Unload driver"
	@echo "  make status   - Show driver status"
	@echo "  make info     - Show RTC information"
	@echo "  make read     - Show RTC devices"
	@echo "  make test     - Test RTC device"
	@echo "  make logs     - Show kernel logs"
