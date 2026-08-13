#!/bin/bash

###############################################################################
# BeagleBone Black - Device Driver Project
#
# Log Collection Script
#
# Collects:
#   - Kernel messages
#   - Device information
#   - Device Tree information
#   - GPIO information
#   - I2C information
#   - SPI information
#   - UART information
#   - PWM information
#   - CAN information
#   - ADC / IIO information
#   - Loaded kernel modules
#   - Kernel configuration
###############################################################################

set -u

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs"
OUTPUT_DIR="${LOG_DIR}/bbb_${TIMESTAMP}"

mkdir -p "${OUTPUT_DIR}"

echo "=========================================="
echo " BeagleBone Black Log Collection"
echo "=========================================="
echo
echo "Output directory:"
echo "${OUTPUT_DIR}"
echo

###############################################################################
# Helper Function
###############################################################################

collect()
{
    NAME="$1"
    COMMAND="$2"

    echo "[INFO] Collecting ${NAME}..."

    {
        echo "============================================================"
        echo "${NAME}"
        echo "============================================================"
        echo
        eval "${COMMAND}"
    } > "${OUTPUT_DIR}/${NAME}.log" 2>&1
}

###############################################################################
# System Information
###############################################################################

collect "system" \
    "uname -a"

collect "kernel_release" \
    "uname -r"

collect "cpuinfo" \
    "cat /proc/cpuinfo"

collect "memory" \
    "cat /proc/meminfo"

collect "uptime" \
    "uptime"

collect "mounts" \
    "mount"

###############################################################################
# Kernel Logs
###############################################################################

collect "dmesg" \
    "dmesg"

collect "dmesg_errors" \
    "dmesg --level=err,warn"

###############################################################################
# Kernel Modules
###############################################################################

collect "loaded_modules" \
    "lsmod"

collect "module_information" \
    "cat /proc/modules"

###############################################################################
# Device Information
###############################################################################

collect "devices" \
    "ls -l /sys/class"

collect "platform_devices" \
    "ls -l /sys/bus/platform/devices"

collect "device_drivers" \
    "find /sys/bus -maxdepth 2 -type d -name drivers"

###############################################################################
# Device Tree
###############################################################################

collect "device_tree" \
    "find /proc/device-tree -maxdepth 3 -type f -print"

collect "device_tree_model" \
    "cat /proc/device-tree/model"

collect "compatible" \
    "tr '\0' '\n' < /proc/device-tree/compatible"

###############################################################################
# GPIO
###############################################################################

collect "gpio" \
    "ls -la /sys/class/gpio"

collect "gpiochips" \
    "ls -la /sys/class/gpiochip"

collect "gpio_devices" \
    "find /sys/bus/gpio/devices -maxdepth 2 -type l -print"

###############################################################################
# I2C
###############################################################################

collect "i2c_adapters" \
    "ls -la /sys/class/i2c-adapter"

collect "i2c_devices" \
    "ls -la /sys/bus/i2c/devices"

collect "i2c_bus" \
    "i2cdetect -l"

###############################################################################
# SPI
###############################################################################

collect "spi_devices" \
    "ls -la /sys/bus/spi/devices"

collect "spi_class" \
    "ls -la /sys/class/spidev"

###############################################################################
# UART
###############################################################################

collect "serial_devices" \
    "ls -la /dev/tty*"

collect "serial_class" \
    "ls -la /sys/class/tty"

collect "serial_driver" \
    "ls -la /sys/bus/serial-base/devices 2>/dev/null || true"

###############################################################################
# PWM
###############################################################################

collect "pwm" \
    "ls -la /sys/class/pwm"

collect "pwm_devices" \
    "find /sys/class/pwm -maxdepth 2 -print"

###############################################################################
# CAN
###############################################################################

collect "can_interfaces" \
    "ip -details link show type can"

collect "network_interfaces" \
    "ip -details link"

collect "can_modules" \
    "lsmod | grep -i can"

###############################################################################
# ADC / IIO
###############################################################################

collect "iio_devices" \
    "ls -la /sys/bus/iio/devices"

collect "iio_device_details" \
    "find /sys/bus/iio/devices -maxdepth 2 -type f -print"

###############################################################################
# USB
###############################################################################

collect "usb" \
    "lsusb"

collect "usb_tree" \
    "lsusb -t"

###############################################################################
# PCI
###############################################################################

collect "pci" \
    "lspci"

###############################################################################
# Interrupts
###############################################################################

collect "interrupts" \
    "cat /proc/interrupts"

###############################################################################
# Memory
###############################################################################

collect "iomem" \
    "cat /proc/iomem"

collect "ioports" \
    "cat /proc/ioports"

###############################################################################
# Kernel Configuration
###############################################################################

if [ -f /proc/config.gz ]; then

    collect "kernel_config" \
        "zcat /proc/config.gz"

elif [ -f "/boot/config-$(uname -r)" ]; then

    collect "kernel_config" \
        "cat /boot/config-$(uname -r)"

else

    echo "[WARNING] Kernel configuration not available."

fi

###############################################################################
# Driver Information
###############################################################################

collect "driver_bindings" \
    "find /sys/bus -type l -path '*/driver' -print"

collect "modules_directory" \
    "find /lib/modules/$(uname -r) -type f 2>/dev/null"

###############################################################################
# Important Device Nodes
###############################################################################

collect "device_nodes" \
    "ls -la /dev/gpiochip* /dev/i2c-* /dev/spidev* /dev/tty* /dev/can* 2>/dev/null || true"

###############################################################################
# Boot Information
###############################################################################

collect "cmdline" \
    "cat /proc/cmdline"

collect "boot_parameters" \
    "cat /proc/cmdline"

###############################################################################
# Date / Time
###############################################################################

collect "date" \
    "date"

###############################################################################
# Create Summary
###############################################################################

cat > "${OUTPUT_DIR}/README.txt" <<EOF
BeagleBone Black Device Driver Log Collection
==============================================

Collection Time:
${TIMESTAMP}

Kernel:
$(uname -r)

Hostname:
$(hostname)

Board:
$(cat /proc/device-tree/model 2>/dev/null || echo "Unknown")

Collected logs:

- system.log
- kernel_release.log
- cpuinfo.log
- memory.log
- uptime.log
- mounts.log
- dmesg.log
- dmesg_errors.log
- loaded_modules.log
- module_information.log
- devices.log
- platform_devices.log
- device_drivers.log
- device_tree.log
- device_tree_model.log
- compatible.log
- gpio.log
- gpiochips.log
- gpio_devices.log
- i2c_adapters.log
- i2c_devices.log
- i2c_bus.log
- spi_devices.log
- spi_class.log
- serial_devices.log
- serial_class.log
- serial_driver.log
- pwm.log
- pwm_devices.log
- can_interfaces.log
- network_interfaces.log
- can_modules.log
- iio_devices.log
- iio_device_details.log
- usb.log
- usb_tree.log
- pci.log
- interrupts.log
- iomem.log
- ioports.log
- kernel_config.log
- driver_bindings.log
- modules_directory.log
- device_nodes.log
- cmdline.log
- boot_parameters.log
- date.log
EOF

###############################################################################
# Create Archive
###############################################################################

ARCHIVE="${LOG_DIR}/bbb_logs_${TIMESTAMP}.tar.gz"

tar -czf "${ARCHIVE}" \
    -C "${LOG_DIR}" \
    "bbb_${TIMESTAMP}"

###############################################################################
# Finish
###############################################################################

echo
echo "=========================================="
echo " Log collection completed"
echo "=========================================="
echo
echo "Logs:"
echo "${OUTPUT_DIR}"
echo
echo "Archive:"
echo "${ARCHIVE}"
echo
