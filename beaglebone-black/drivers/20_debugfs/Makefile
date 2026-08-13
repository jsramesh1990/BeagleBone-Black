###############################################################################
# BeagleBone Black - Debugfs Driver Demo
#
# Output:
#     debugfs_driver.ko
###############################################################################

obj-m += debugfs_driver.o


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
	sudo insmod debugfs_driver.ko


###############################################################################
# Unload
###############################################################################

.PHONY: unload

unload:
	sudo rmmod debugfs_driver


###############################################################################
# Status
###############################################################################

.PHONY: status

status:
	@echo "Debugfs driver:"
	@lsmod | grep debugfs_driver || true
	@echo ""
	@echo "Debugfs directory:"
	@ls -la /sys/kernel/debug/bbb_debugfs 2>/dev/null || true


###############################################################################
# Logs
###############################################################################

.PHONY: logs

logs:
	dmesg | grep -i debugfs


###############################################################################
# Test

.PHONY: test

test:
	@echo "Debugfs driver test"
	@echo ""
	@echo "Directory:"
	@ls -la /sys/kernel/debug/bbb_debugfs 2>/dev/null || true
	@echo ""
	@echo "Status:"
	@cat /sys/kernel/debug/bbb_debugfs/status 2>/dev/null || true


###############################################################################
# Help
###############################################################################

.PHONY: help

help:
	@echo "BeagleBone Black Debugfs Driver"
	@echo ""
	@echo "  make          - Build driver"
	@echo "  make clean    - Clean build"
	@echo "  make load     - Load driver"
	@echo "  make unload   - Unload driver"
	@echo "  make status   - Show driver status"
	@echo "  make logs     - Show kernel logs"
	@echo "  make test     - Run debugfs test"
