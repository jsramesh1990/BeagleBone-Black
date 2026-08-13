### `beaglebone-black/drivers/05_spi/Makefile`

```makefile
###############################################################################
# BeagleBone Black - SPI Drivers Makefile
#
# Drivers:
#   spi_display.c
#   spi_sensor.c
###############################################################################

obj-m += spi_display.o
obj-m += spi_sensor.o

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
# Load Drivers
###############################################################################

.PHONY: load

load: all
	sudo insmod spi_display.ko
	sudo insmod spi_sensor.ko


###############################################################################
# Remove Drivers
###############################################################################

.PHONY: unload

unload:
	-sudo rmmod spi_sensor
	-sudo rmmod spi_display


###############################################################################
# Driver Status
###############################################################################

.PHONY: status

status:
	@echo "SPI drivers:"
	@lsmod | grep -E "spi_display|spi_sensor" || true
	@echo ""
	@echo "SPI device nodes:"
	@ls -l /dev/bbb_spi_* 2>/dev/null || true


###############################################################################
# SPI Devices
###############################################################################

.PHONY: devices

devices:
	@ls -l /sys/bus/spi/devices/


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
	@echo "SPI Driver Makefile"
	@echo ""
	@echo "  make              - Build drivers"
	@echo "  make clean        - Clean build"
	@echo "  make load         - Load display and sensor drivers"
	@echo "  make unload       - Remove display and sensor drivers"
	@echo "  make status       - Show driver status"
	@echo "  make devices      - Show SPI devices"
	@echo "  make logs         - Show kernel logs"
```

Directory:

```text
beaglebone-black/drivers/05_spi/
├── spi_display.c
├── spi_display.h
├── spi_sensor.c
├── spi_sensor.h
└── Makefile
```

Build:

```bash
cd beaglebone-black/drivers/05_spi
make
```

Expected modules:

```text
spi_display.ko
spi_sensor.ko
```

Load:

```bash
make load
```

Check:

```bash
make status
make devices
make logs
```

Remove:

```bash
make unload
```

Clean:

```bash
make clean
```

