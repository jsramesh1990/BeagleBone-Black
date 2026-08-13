###############################################################################
# BeagleBone Black - ADC Driver Makefile
#
# Driver:
#     adc_driver.c
#
# Output:
#     adc_driver.ko
###############################################################################

obj-m += adc_driver.o


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
	sudo insmod adc_driver.ko


###############################################################################
# Unload
###############################################################################

.PHONY: unload

unload:
	sudo rmmod adc_driver


###############################################################################
# Status
###############################################################################

.PHONY: status

status:
	@echo "ADC driver:"
	@lsmod | grep adc_driver || true
	@echo ""
	@echo "IIO devices:"
	@ls -l /sys/bus/iio/devices/ 2>/dev/null || true


###############################################################################
# ADC Information
###############################################################################

.PHONY: info

info:
	@echo "ADC IIO devices:"
	@for dev in /sys/bus/iio/devices/iio:device*; do \
		if [ -d "$$dev" ]; then \
			echo "----------------------------------------"; \
			echo "$$dev"; \
			cat "$$dev/name" 2>/dev/null || true; \
		fi; \
	done


###############################################################################
# Read ADC Channels
###############################################################################

.PHONY: read

read:
	@for file in /sys/bus/iio/devices/iio:device*/in_voltage*_raw; do \
		if [ -f "$$file" ]; then \
			echo -n "$$file = "; \
			cat "$$file"; \
		fi; \
	done


###############################################################################
# Kernel Logs
###############################################################################

.PHONY: logs

logs:
	dmesg | grep -i adc


###############################################################################
# Test
###############################################################################

.PHONY: test

test:
	@echo "Checking ADC IIO interface..."
	@if ls /sys/bus/iio/devices/iio:device*/in_voltage*_raw >/dev/null 2>&1; then \
		echo "ADC channels detected."; \
	else \
		echo "No ADC channels detected."; \
	fi


###############################################################################
# Help
###############################################################################

.PHONY: help

help:
	@echo "BeagleBone Black ADC Driver"
	@echo ""
	@echo "  make          - Build driver"
	@echo "  make clean    - Clean build"
	@echo "  make load     - Load driver"
	@echo "  make unload   - Unload driver"
	@echo "  make status   - Show driver status"
	@echo "  make info     - Show IIO ADC information"
	@echo "  make read     - Read ADC raw values"
	@echo "  make test     - Test ADC IIO interface"
	@echo "  make logs     - Show ADC kernel logs"
