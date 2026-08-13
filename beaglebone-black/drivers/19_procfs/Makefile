###############################################################################
# BeagleBone Black - Procfs Driver Demo
#
# Output:
#     proc_driver.ko
###############################################################################

obj-m += proc_driver.o


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
	sudo insmod proc_driver.ko


###############################################################################
# Unload
###############################################################################

.PHONY: unload

unload:
	sudo rmmod proc_driver


###############################################################################
# Status
###############################################################################

.PHONY: status

status:
	@echo "Procfs driver:"
	@lsmod | grep proc_driver || true
	@echo ""
	@echo "Procfs entries:"
	@ls -la /proc/bbb_proc 2>/dev/null || true


###############################################################################
# Logs
###############################################################################

.PHONY: logs

logs:
	dmesg | grep -i "procfs"


###############################################################################
# Test

.PHONY: test

test:
	@echo "Procfs driver test"
	@echo ""
	@echo "Directory:"
	@ls -la /proc/bbb_proc 2>/dev/null || true
	@echo ""
	@echo "Status:"
	@cat /proc/bbb_proc/status 2>/dev/null || true


###############################################################################
# Help
###############################################################################

.PHONY: help

help:
	@echo "BeagleBone Black Procfs Driver"
	@echo ""
	@echo "  make          - Build driver"
	@echo "  make clean    - Clean build"
	@echo "  make load     - Load driver"
	@echo "  make unload   - Unload driver"
	@echo "  make status   - Show driver status"
	@echo "  make logs     - Show kernel logs"
	@echo "  make test     - Run procfs test"
