###############################################################################
# BeagleBone Black - GPIO Interrupt Driver Makefile
#
# Driver:
#     gpio_irq.c
#
# Output:
#     gpio_irq.ko
###############################################################################


###############################################################################
# Kernel Module
###############################################################################

obj-m += gpio_irq.o


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
# Load Driver
###############################################################################

.PHONY: load

load: all
	sudo insmod gpio_irq.ko


###############################################################################
# Unload Driver
###############################################################################

.PHONY: unload

unload:
	sudo rmmod gpio_irq


###############################################################################
# Driver Status
###############################################################################

.PHONY: status

status:
	@echo "GPIO IRQ module:"
	@lsmod | grep gpio_irq || true
	@echo ""
	@echo "Device:"
	@ls -l /dev/bbb_gpio_irq 2>/dev/null || true


###############################################################################
# IRQ Information
###############################################################################

.PHONY: irq

irq:
	@echo "GPIO IRQ information:"
	@cat /proc/interrupts | grep -i gpio || true


###############################################################################
# Kernel Logs
###############################################################################

.PHONY: logs

logs:
	dmesg | tail -40


###############################################################################
# Test Device
###############################################################################

.PHONY: test

test:
	@if [ -e /dev/bbb_gpio_irq ]; then \
		echo "GPIO IRQ device exists: /dev/bbb_gpio_irq"; \
	else \
		echo "ERROR: /dev/bbb_gpio_irq does not exist"; \
	fi


###############################################################################
# Help
###############################################################################

.PHONY: help

help:
	@echo "BeagleBone Black GPIO Interrupt Driver"
	@echo ""
	@echo "Build:"
	@echo "  make"
	@echo ""
	@echo "Load:"
	@echo "  make load"
	@echo ""
	@echo "Unload:"
	@echo "  make unload"
	@echo ""
	@echo "Status:"
	@echo "  make status"
	@echo ""
	@echo "IRQ information:"
	@echo "  make irq"
	@echo ""
	@echo "Kernel logs:"
	@echo "  make logs"
	@echo ""
	@echo "Test:"
	@echo "  make test"
	@echo ""
	@echo "Clean:"
	@echo "  make clean"
