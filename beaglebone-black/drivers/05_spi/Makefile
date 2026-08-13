###############################################################################
# BeagleBone Black - SPI Driver Makefile
#
# Drivers:
#     spi_display.c
#     spi_sensor.c
#
# Output:
#     spi_display.ko
#     spi_sensor.ko
###############################################################################


###############################################################################
# Kernel Modules
###############################################################################

obj-m += spi_display.o
obj-m += spi_sensor.o


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
# Load Display Driver
###############################################################################

.PHONY: load-display

load-display: all
	sudo insmod spi_display.ko


###############################################################################
# Load Sensor Driver
###############################################################################

.PHONY: load-sensor

load-sensor: all
	sudo insmod spi_sensor.ko


###############################################################################
# Unload Display Driver
###############################################################################

.PHONY: unload-display

unload-display:
	sudo rmmod spi_display


###############################################################################
# Unload Sensor Driver
###############################################################################

.PHONY: unload-sensor

unload-sensor:
	sudo rmmod spi_sensor


###############################################################################
# Status
###############################################################################

.PHONY: status

status:
	@echo "SPI modules:"
	@lsmod | grep -E "spi_display|spi_sensor" || true
	@echo ""
	@echo "SPI device nodes:"
	@ls -l /dev/bbb_spi_* 2>/dev/null || true


###############################################################################
# SPI Devices
###############################################################################

.PHONY: devices

devices:
	@echo "SPI devices:"
	@ls -l /sys/bus/spi/devices/ 2>/dev/null || true


###############################################################################
# Kernel Logs
###############################################################################

.PHONY: logs

logs:
	dmesg | tail -50


###############################################################################
# Build and Load Both Drivers
###############################################################################

.PHONY: load

load: all
	sudo insmod spi_display.ko
	sudo insmod spi_sensor.ko


###############################################################################
# Unload Both Drivers
###############################################################################

.PHONY: unload

unload:
	-sudo rmmod spi_sensor
	-sudo rmmod spi_display


###############################################################################
# Help
###############################################################################

.PHONY: help

help:
	@echo "BeagleBone Black SPI Drivers"
	@echo ""
	@echo "Build:"
	@echo "  make"
	@echo ""
	@echo "Load display:"
	@echo "  make load-display"
	@echo ""
	@echo "Load sensor:"
	@echo "  make load-sensor"
	@echo ""
	@echo "Load both:"
	@echo "  make load"
	@echo ""
	@echo "Unload display:"
	@echo "  make unload-display"
	@echo ""
	@echo "Unload sensor:"
	@echo "  make unload-sensor"
	@echo ""
	@echo "Unload both:"
	@echo "  make unload"
	@echo ""
	@echo "Status:"
	@echo "  make status"
	@echo ""
	@echo "SPI devices:"
	@echo "  make devices"
	@echo ""
	@echo "Logs:"
	@echo "  make logs"
	@echo ""
	@echo "Clean:"
	@echo "  make clean"
