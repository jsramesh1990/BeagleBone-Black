###############################################################################
# BeagleBone Black - DMA Driver Makefile
#
# Output:
#     dma_driver.ko
###############################################################################

obj-m += dma_driver.o


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
	sudo insmod dma_driver.ko


###############################################################################
# Unload
###############################################################################

.PHONY: unload

unload:
	sudo rmmod dma_driver


###############################################################################
# Status
###############################################################################

.PHONY: status

status:
	@echo "DMA driver:"
	@lsmod | grep dma_driver || true

	@echo ""
	@echo "DMA devices:"
	@ls -l /sys/class/dma/ 2>/dev/null || true


###############################################################################
# Logs
###############################################################################

.PHONY: logs

logs:
	dmesg | grep -i "dma"


###############################################################################
# Test

.PHONY: test

test:
	@echo "DMA driver test"
	@echo ""
	@echo "Check DMA channels:"
	@ls -l /sys/class/dma/ 2>/dev/null || true
	@echo ""
	@echo "Check kernel logs:"
	@dmesg | grep -i dma | tail -20


###############################################################################
# Help
###############################################################################

.PHONY: help

help:
	@echo "BeagleBone Black DMA Driver"
	@echo ""
	@echo "  make          - Build driver"
	@echo "  make clean    - Clean build"
	@echo "  make load     - Load driver"
	@echo "  make unload   - Unload driver"
	@echo "  make status   - Show DMA status"
	@echo "  make logs     - Show kernel logs"
	@echo "  make test     - Test DMA"
