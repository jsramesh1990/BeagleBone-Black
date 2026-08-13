#!/bin/bash

###############################################################################
# BeagleBone Black - Device Scan Utility
#
# File:
#   tools/device_scan.sh
#
# Purpose:
#   Scan and display available BBB peripherals, device nodes, buses,
#   GPIO controllers, I2C devices, SPI devices, UART devices, PWM devices,
#   CAN interfaces, USB devices, and relevant kernel information.
#
# Usage:
#   sudo ./device_scan.sh
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs"
LOG_FILE="${LOG_DIR}/device_scan_${TIMESTAMP}.log"

mkdir -p "${LOG_DIR}"

###############################################################################
# Logging
###############################################################################

log()
{
    echo "$1" | tee -a "${LOG_FILE}"
}

section()
{
    log ""
    log "============================================================"
    log " $1"
    log "============================================================"
}

###############################################################################
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - Device Scan"
log "============================================================"
log "Date       : $(date)"
log "Hostname   : $(hostname)"
log "Kernel     : $(uname -r)"
log "Architecture: $(uname -m)"
log "Log File   : ${LOG_FILE}"
log "============================================================"

###############################################################################
# System Information
###############################################################################

section "SYSTEM INFORMATION"

log "Kernel:"
uname -a | tee -a "${LOG_FILE}"

log ""
log "OS Information:"

if [ -f /etc/os-release ]; then
    cat /etc/os-release | tee -a "${LOG_FILE}"
else
    log "OS information unavailable."
fi

###############################################################################
# Device Tree
###############################################################################

section "DEVICE TREE"

if [ -d /sys/firmware/devicetree/base ]; then

    log "Device Tree is available."

    log ""
    log "Compatible:"
    tr '\0' '\n' < /sys/firmware/devicetree/base/compatible \
        2>/dev/null |
        tee -a "${LOG_FILE}" || true

else

    log "Device Tree filesystem unavailable."

fi

###############################################################################
# GPIO
###############################################################################

section "GPIO DEVICES"

if [ -d /sys/class/gpio ]; then

    log "GPIO sysfs:"
    ls -la /sys/class/gpio 2>/dev/null |
        tee -a "${LOG_FILE}" || true

fi

log ""
log "GPIO character devices:"

if ls /dev/gpiochip* >/dev/null 2>&1; then

    ls -l /dev/gpiochip* |
        tee -a "${LOG_FILE}"

else

    log "No /dev/gpiochip devices found."

fi

if [ -f /sys/kernel/debug/gpio ]; then

    log ""
    log "GPIO controller state:"
    cat /sys/kernel/debug/gpio 2>/dev/null |
        tee -a "${LOG_FILE}" || true

fi

###############################################################################
# I2C
###############################################################################

section "I2C DEVICES"

if command -v i2cdetect >/dev/null 2>&1; then

    log "I2C adapters:"
    i2cdetect -l |
        tee -a "${LOG_FILE}"

else

    log "i2cdetect is not installed."

fi

log ""
log "I2C device nodes:"

ls -l /dev/i2c-* 2>/dev/null |
    tee -a "${LOG_FILE}" || true

if [ -d /sys/class/i2c-adapter ]; then

    log ""
    log "I2C sysfs adapters:"

    ls -la /sys/class/i2c-adapter |
        tee -a "${LOG_FILE}" || true

fi

###############################################################################
# SPI
###############################################################################

section "SPI DEVICES"

log "SPI device nodes:"

if ls /dev/spidev* >/dev/null 2>&1; then

    ls -l /dev/spidev* |
        tee -a "${LOG_FILE}"

else

    log "No SPI spidev devices found."

fi

if [ -d /sys/class/spidev ]; then

    log ""
    log "SPI sysfs devices:"

    ls -la /sys/class/spidev |
        tee -a "${LOG_FILE}" || true

fi

###############################################################################
# UART
###############################################################################

section "UART / SERIAL DEVICES"

log "Serial devices:"

ls -l \
    /dev/ttyS* \
    /dev/ttyO* \
    /dev/ttyAMA* \
    /dev/ttyUSB* \
    /dev/ttyACM* \
    2>/dev/null |
    tee -a "${LOG_FILE}" || true

log ""
log "Serial aliases:"

ls -l /dev/serial/by-id 2>/dev/null |
    tee -a "${LOG_FILE}" || true

###############################################################################
# PWM
###############################################################################

section "PWM DEVICES"

if [ -d /sys/class/pwm ]; then

    log "PWM controllers:"

    ls -la /sys/class/pwm |
        tee -a "${LOG_FILE}" || true

    log ""

    for PWMCHIP in /sys/class/pwm/pwmchip*; do

        [ -d "${PWMCHIP}" ] || continue

        log "Controller: ${PWMCHIP}"

        if [ -f "${PWMCHIP}/npwm" ]; then
            log "Channels: $(cat "${PWMCHIP}/npwm")"
        fi

        if [ -f "${PWMCHIP}/device/driver/module" ]; then
            log "Driver:"
            readlink -f "${PWMCHIP}/device/driver/module" |
                tee -a "${LOG_FILE}" || true
        fi

    done

else

    log "PWM subsystem unavailable."

fi

###############################################################################
# CAN
###############################################################################

section "CAN DEVICES"

if command -v ip >/dev/null 2>&1; then

    log "CAN network interfaces:"

    ip -details link show type can 2>/dev/null |
        tee -a "${LOG_FILE}" || true

    log ""
    log "All network interfaces:"

    ip -br link |
        tee -a "${LOG_FILE}"

else

    log "ip command is unavailable."

fi

###############################################################################
# Ethernet
###############################################################################

section "ETHERNET DEVICES"

if command -v ip >/dev/null 2>&1; then

    ip -br link |
        tee -a "${LOG_FILE}"

    log ""
    log "Ethernet interfaces:"

    for IFACE in /sys/class/net/*; do

        NAME="$(basename "${IFACE}")"

        if [ -e "${IFACE}/device" ]; then

            log ""
            log "Interface: ${NAME}"

            if [ -f "${IFACE}/address" ]; then
                log "MAC: $(cat "${IFACE}/address")"
            fi

            if [ -f "${IFACE}/operstate" ]; then
                log "State: $(cat "${IFACE}/operstate")"
            fi

        fi

    done

fi

###############################################################################
# USB
###############################################################################

section "USB DEVICES"

if command -v lsusb >/dev/null 2>&1; then

    lsusb |
        tee -a "${LOG_FILE}"

else

    log "lsusb is not installed."

fi

###############################################################################
# PCIe
###############################################################################

section "PCIe DEVICES"

if command -v lspci >/dev/null 2>&1; then

    lspci |
        tee -a "${LOG_FILE}"

else

    log "lspci is not installed."

fi

###############################################################################
# MMC / eMMC / SD
###############################################################################

section "MMC / eMMC / SD DEVICES"

if [ -d /sys/class/mmc_host ]; then

    find /sys/class/mmc_host \
        -maxdepth 3 \
        -type d \
        2>/dev/null |
        tee -a "${LOG_FILE}"

else

    log "MMC subsystem information unavailable."

fi

###############################################################################
# Block Devices
###############################################################################

section "BLOCK DEVICES"

if command -v lsblk >/dev/null 2>&1; then

    lsblk -o \
        NAME,MAJ:MIN,SIZE,TYPE,FSTYPE,MOUNTPOINT |
        tee -a "${LOG_FILE}"

else

    log "lsblk is unavailable."

fi

###############################################################################
# Kernel Modules
###############################################################################

section "LOADED KERNEL MODULES"

if command -v lsmod >/dev/null 2>&1; then

    lsmod |
        tee -a "${LOG_FILE}"

else

    log "lsmod is unavailable."

fi

###############################################################################
# Drivers
###############################################################################

section "DEVICE DRIVERS"

log "Loaded driver information:"

if [ -d /sys/bus ]; then

    for BUS in \
        gpio \
        i2c \
        spi \
        serial \
        pwm \
        net \
        usb \
        platform
    do

        if [ -d "/sys/bus/${BUS}/drivers" ]; then

            log ""
            log "Bus: ${BUS}"

            find "/sys/bus/${BUS}/drivers" \
                -mindepth 1 \
                -maxdepth 1 \
                -type d \
                -printf "%f\n" \
                2>/dev/null |
                sort |
                tee -a "${LOG_FILE}" || true

        fi

    done

fi

###############################################################################
# Interrupts
###############################################################################

section "INTERRUPT INFORMATION"

if [ -f /proc/interrupts ]; then

    log "Peripheral-related interrupts:"

    grep -Ei \
        "gpio|i2c|spi|uart|serial|pwm|can|mmc|usb|eth|eqep" \
        /proc/interrupts |
        tee -a "${LOG_FILE}" || true

fi

###############################################################################
# DMA
###############################################################################

section "DMA INFORMATION"

if [ -d /sys/class/dma ]; then

    ls -la /sys/class/dma |
        tee -a "${LOG_FILE}" || true

else

    log "DMA sysfs information unavailable."

fi

###############################################################################
# Kernel Messages
###############################################################################

section "RECENT PERIPHERAL KERNEL MESSAGES"

dmesg 2>/dev/null |
    grep -iE \
    "gpio|i2c|spi|uart|serial|pwm|can|ethernet|usb|mmc|dma|"
    "pinctrl|pinmux|driver|probe|error|failed" |
    tail -100 |
    tee -a "${LOG_FILE}" || true

###############################################################################
# Device Node Summary
###############################################################################

section "DEVICE NODE SUMMARY"

log "GPIO:"
ls /dev/gpiochip* 2>/dev/null |
    tee -a "${LOG_FILE}" || true

log ""
log "I2C:"
ls /dev/i2c-* 2>/dev/null |
    tee -a "${LOG_FILE}" || true

log ""
log "SPI:"
ls /dev/spidev* 2>/dev/null |
    tee -a "${LOG_FILE}" || true

log ""
log "UART:"
ls \
    /dev/ttyS* \
    /dev/ttyO* \
    /dev/ttyAMA* \
    /dev/ttyUSB* \
    /dev/ttyACM* \
    2>/dev/null |
    tee -a "${LOG_FILE}" || true

###############################################################################
# Final Summary
###############################################################################

section "SCAN COMPLETE"

log "BeagleBone Black peripheral scan completed."

log ""
log "Results saved to:"
log "${LOG_FILE}"

log ""
log "============================================================"
log " END OF DEVICE SCAN"
log "============================================================"

exit 0
