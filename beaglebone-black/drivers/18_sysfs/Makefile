###############################################################################
# BeagleBone Black - Sysfs Driver Demo
#
# Output:
#     sysfs_driver.ko
###############################################################################

obj-m += sysfs_driver.o


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
	sudo insmod sysfs_driver.ko


###############################################################################
# Unload
###############################################################################

.PHONY: unload

unload:
	sudo rmmod sysfs_driver


###############################################################################
# Status
###############################################################################

.PHONY: status

status:
	@echo "Sysfs driver:"
	@lsmod | grep sysfs_driver || true

	@echo ""
	@echo "Driver sysfs entries:"
	@find /sys/bus/platform/devices \
		-maxdepth 2 \
		-type d \
		-name "*sysfs*" \
		2>/dev/null || true


###############################################################################
# Logs
###############################################################################

.PHONY: logs

logs:
	dmesg | grep -i "sysfs"


###############################################################################
# Test

.PHONY: test

test:
	@echo "Sysfs driver test"
	@echo ""
	@echo "Search for driver:"
	@find /sys/bus/platform/devices \
		-maxdepth 2 \
		-type f \
		\( -name "value" -o -name "enable" -o -name "status" \) \
		2>/dev/null || true


###############################################################################
# Help
###############################################################################

.PHONY: help

help:
	@echo "BeagleBone Black Sysfs Driver"
	@echo ""
	@echo "  make          - Build driver"
	@echo "  make clean    - Clean build"
	@echo "  make load     - Load driver"
	@echo "  make unload   - Unload driver"
	@echo "  make status   - Show driver status"
	@echo "  make logs     - Show kernel logs"
	@echo "  make test     - Search sysfs attributes"
