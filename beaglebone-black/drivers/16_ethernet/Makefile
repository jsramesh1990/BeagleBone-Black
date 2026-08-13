###############################################################################
# BeagleBone Black - Ethernet Driver Demo
#
# Output:
#     ethernet_demo.ko
###############################################################################

obj-m += ethernet_demo.o


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
	sudo insmod ethernet_demo.ko


###############################################################################
# Unload
###############################################################################

.PHONY: unload

unload:
	sudo rmmod ethernet_demo


###############################################################################
# Status

.PHONY: status

status:
	@echo "Ethernet driver:"
	@lsmod | grep ethernet_demo || true
	@echo ""
	@echo "Network interfaces:"
	@ip link show


###############################################################################
# Logs

.PHONY: logs

logs:
	dmesg | grep -i ethernet


###############################################################################
# Interface Information

.PHONY: info

info:
	@ip -br link
	@echo ""
	@ip -br addr


###############################################################################
# Test

.PHONY: test

test:
	@echo "Ethernet driver test"
	@echo ""
	@echo "Interfaces:"
	@ip -br link
	@echo ""
	@echo "Statistics:"
	@ip -s link


###############################################################################
# Help

.PHONY: help

help:
	@echo "BeagleBone Black Ethernet Driver"
	@echo ""
	@echo "  make          - Build driver"
	@echo "  make clean    - Clean build"
	@echo "  make load     - Load driver"
	@echo "  make unload   - Unload driver"
	@echo "  make status   - Driver/interface status"
	@echo "  make logs     - Show Ethernet logs"
	@echo "  make info     - Interface information"
	@echo "  make test     - Show network statistics"
