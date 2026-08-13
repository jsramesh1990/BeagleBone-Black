###############################################################################
# BeagleBone Black - I2C Driver Makefile
#
# Drivers:
#     i2c_eeprom.c
#     i2c_sensor.c
#
# Output:
#     i2c_eeprom.ko
#     i2c_sensor.ko
###############################################################################


###############################################################################
# Kernel Modules
###############################################################################

obj-m += i2c_eeprom.o
obj-m += i2c_sensor.o


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
# Load EEPROM Driver
###############################################################################

.PHONY: load-eeprom

load-eeprom: all
	sudo insmod i2c_eeprom.ko


###############################################################################
# Load Sensor Driver
###############################################################################

.PHONY: load-sensor

load-sensor: all
	sudo insmod i2c_sensor.ko


###############################################################################
# Unload EEPROM Driver
###############################################################################

.PHONY: unload-eeprom

unload-eeprom:
	sudo rmmod i2c_eeprom


###############################################################################
# Unload Sensor Driver
###############################################################################

.PHONY: unload-sensor

unload-sensor:
	sudo rmmod i2c_sensor


###############################################################################
# Status
###############################################################################

.PHONY: status

status:
	@echo "I2C modules:"
	@lsmod | grep -E "i2c_eeprom|i2c_sensor" || true
	@echo ""
	@echo "I2C device nodes:"
	@ls -l /dev/bbb_i2c_* 2>/dev/null || true


###############################################################################
# I2C Bus Scan
###############################################################################

.PHONY: scan

scan:
	sudo i2cdetect -y 2


###############################################################################
# Kernel Logs
###############################################################################

.PHONY: logs

logs:
	dmesg | tail -50


###############################################################################
# Help
###############################################################################

.PHONY: help

help:
	@echo "BeagleBone Black I2C Drivers"
	@echo ""
	@echo "Build:"
	@echo "  make"
	@echo ""
	@echo "Load EEPROM:"
	@echo "  make load-eeprom"
	@echo ""
	@echo "Load Sensor:"
	@echo "  make load-sensor"
	@echo ""
	@echo "Unload EEPROM:"
	@echo "  make unload-eeprom"
	@echo ""
	@echo "Unload Sensor:"
	@echo "  make unload-sensor"
	@echo ""
	@echo "Status:"
	@echo "  make status"
	@echo ""
	@echo "Scan I2C:"
	@echo "  make scan"
	@echo ""
	@echo "Logs:"
	@echo "  make logs"
	@echo ""
	@echo "Clean:"
	@echo "  make clean"
