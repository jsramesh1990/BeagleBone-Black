###############################################################################
# BeagleBone Black - ALSA Driver Demo
#
# Output:
#     alsa_driver.ko
###############################################################################

obj-m += alsa_driver.o


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
	sudo insmod alsa_driver.ko


###############################################################################
# Unload
###############################################################################

.PHONY: unload

unload:
	sudo rmmod alsa_driver


###############################################################################
# Status
###############################################################################

.PHONY: status

status:
	@echo "ALSA module:"
	@lsmod | grep alsa_driver || true
	@echo ""
	@echo "Sound cards:"
	@cat /proc/asound/cards 2>/dev/null || true


###############################################################################
# ALSA Information
###############################################################################

.PHONY: info

info:
	@echo "ALSA cards:"
	@cat /proc/asound/cards 2>/dev/null || true
	@echo ""
	@echo "PCM devices:"
	@cat /proc/asound/pcm 2>/dev/null || true


###############################################################################
# Kernel Logs
###############################################################################

.PHONY: logs

logs:
	dmesg | grep -i alsa


###############################################################################
# Test
###############################################################################

.PHONY: test

test:
	@echo "ALSA driver test"
	@echo ""
	@echo "Sound cards:"
	@cat /proc/asound/cards 2>/dev/null || true
	@echo ""
	@echo "PCM devices:"
	@cat /proc/asound/pcm 2>/dev/null || true
	@echo ""
	@echo "ALSA utilities:"
	@which aplay 2>/dev/null || true
	@which arecord 2>/dev/null || true


###############################################################################
# Help
###############################################################################

.PHONY: help

help:
	@echo "BeagleBone Black ALSA Driver"
	@echo ""
	@echo "  make          - Build driver"
	@echo "  make clean    - Clean build"
	@echo "  make load     - Load driver"
	@echo "  make unload   - Unload driver"
	@echo "  make status   - Show ALSA status"
	@echo "  make info     - Show ALSA devices"
	@echo "  make logs     - Show kernel logs"
	@echo "  make test     - Run basic test"
