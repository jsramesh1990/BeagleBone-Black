###############################################################################
# BeagleBone Black - GPIO Driver Makefile
#
# Drivers:
#     gpio_button.c
#     gpio_led.c
###############################################################################

obj-m += gpio_button.o
obj-m += gpio_led.o


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
# Load GPIO LED Driver
###############################################################################

.PHONY: load-led

load-led: all
	sudo insmod gpio_led.ko


###############################################################################
# Load GPIO Button Driver
###############################################################################

.PHONY: load-button

load-button: all
	sudo insmod gpio_button.ko


###############################################################################
# Remove GPIO LED Driver
###############################################################################

.PHONY: unload-led

unload-led:
	sudo rmmod gpio_led


###############################################################################
# Remove GPIO Button Driver
###############################################################################

.PHONY: unload-button

unload-button:
	sudo rmmod gpio_button


###############################################################################
# Status
###############################################################################

.PHONY: status

status:
	@echo "Loaded GPIO modules:"
	@lsmod | grep -E "gpio_button|gpio_led" || true
	@echo ""
	@echo "GPIO devices:"
	@ls -l /dev/bbb_gpio_* 2>/dev/null || true


###############################################################################
# Kernel Logs
###############################################################################

.PHONY: logs

logs:
	dmesg | tail -40


###############################################################################
# Help
###############################################################################

.PHONY: help

help:
	@echo "BeagleBone Black GPIO Driver"
	@echo ""
	@echo "Build:"
	@echo "  make"
	@echo ""
	@echo "Load:"
	@echo "  make load-led"
	@echo "  make load-button"
	@echo ""
	@echo "Unload:"
	@echo "  make unload-led"
	@echo "  make unload-button"
	@echo ""
	@echo "Status:"
	@echo "  make status"
	@echo ""
	@echo "Logs:"
	@echo "  make logs"
	@echo ""
	@echo "Clean:"
	@echo "  make clean"
