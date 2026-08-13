###############################################################################
# BeagleBone Black - PWM Driver Makefile
#
# Driver:
#     pwm_driver.c
#
# Output:
#     pwm_driver.ko
###############################################################################

obj-m += pwm_driver.o


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
	sudo insmod pwm_driver.ko


###############################################################################
# Unload
###############################################################################

.PHONY: unload

unload:
	sudo rmmod pwm_driver


###############################################################################
# Status
###############################################################################

.PHONY: status

status:
	@echo "PWM driver:"
	@lsmod | grep pwm_driver || true
	@echo ""
	@echo "PWM devices:"
	@ls -l /sys/class/pwm/ 2>/dev/null || true


###############################################################################
# PWM Information
###############################################################################

.PHONY: info

info:
	@echo "PWM chips:"
	@ls -l /sys/class/pwm/ 2>/dev/null || true


###############################################################################
# Kernel Logs
###############################################################################

.PHONY: logs

logs:
	dmesg | grep -i pwm


###############################################################################
# Test
###############################################################################

.PHONY: test

test:
	@echo "Checking PWM framework..."
	@if [ -d /sys/class/pwm ]; then \
		echo "PWM framework available."; \
	else \
		echo "PWM framework not available."; \
	fi


###############################################################################
# Help
###############################################################################

.PHONY: help

help:
	@echo "BeagleBone Black PWM Driver"
	@echo ""
	@echo "  make          - Build driver"
	@echo "  make clean    - Clean build"
	@echo "  make load     - Load driver"
	@echo "  make unload   - Unload driver"
	@echo "  make status   - Show driver status"
	@echo "  make info     - Show PWM information"
	@echo "  make test     - Test PWM framework"
	@echo "  make logs     - Show kernel logs"
