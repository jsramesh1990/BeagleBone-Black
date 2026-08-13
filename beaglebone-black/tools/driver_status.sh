#!/bin/bash

###############################################################################
# BeagleBone Black - Driver Status Utility
#
# File:
#   tools/driver_status.sh
#
# Purpose:
#   Display the status of major BeagleBone Black peripheral drivers including
#   GPIO, I2C, SPI, UART, PWM, CAN, Ethernet, USB, MMC and Device Tree.
#
# Usage:
#   sudo ./driver_status.sh
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs"
LOG_FILE="${LOG_DIR}/driver_status_${TIMESTAMP}.log"

mkdir -p "${LOG_DIR}"

###############################################################################
# Counters
###############################################################################

PASS=0
FAIL=0
WARN=0

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

pass()
{
    echo "[PASS] $1" | tee -a "${LOG_FILE}"
    PASS=$((PASS + 1))
}

fail()
{
    echo "[FAIL] $1" | tee -a "${LOG_FILE}"
    FAIL=$((FAIL + 1))
}

warn()
{
    echo "[WARN] $1" | tee -a "${LOG_FILE}"
    WARN=$((WARN + 1))
}

###############################################################################
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - Driver Status"
log "============================================================"
log "Date        : $(date)"
log "Hostname    : $(hostname)"
log "Kernel      : $(uname -r)"
log "Architecture: $(uname -m)"
log "Log File    : ${LOG_FILE}"
log "============================================================"

###############################################################################
# Root Check
###############################################################################

if [ "${EUID}" -ne 0 ]; then
    warn "Running without root privileges. Some driver information may be limited."
fi

###############################################################################
# Kernel Status
###############################################################################

section "KERNEL STATUS"

log "Kernel:"
uname -a | tee -a "${LOG_FILE}"

if [ -d /lib/modules/"$(uname -r)" ]; then

    pass "Kernel module directory exists."

else

    fail "Kernel module directory not found."

fi

###############################################################################
# Device Tree Status
###############################################################################

section "DEVICE TREE STATUS"

if [ -d /sys/firmware/devicetree/base ]; then

    pass "Device Tree is mounted."

    log ""
    log "Compatible:"
    tr '\0' '\n' < /sys/firmware/devicetree/base/compatible \
        2>/dev/null |
        tee -a "${LOG_FILE}" || true

else

    fail "Device Tree filesystem is unavailable."

fi

###############################################################################
# GPIO DRIVER
###############################################################################

section "GPIO DRIVER"

if [ -d /sys/class/gpio ]; then

    pass "GPIO sysfs subsystem available."

else

    warn "GPIO sysfs interface unavailable."

fi

if ls /dev/gpiochip* >/dev/null 2>&1; then

    log "GPIO character devices:"
    ls -l /dev/gpiochip* |
        tee -a "${LOG_FILE}"

    pass "GPIO character-device interface available."

else

    warn "No GPIO character devices found."

fi

if [ -f /sys/kernel/debug/gpio ]; then

    log ""
    log "GPIO controller status:"
    cat /sys/kernel/debug/gpio 2>/dev/null |
        tee -a "${LOG_FILE}" || true

fi

###############################################################################
# I2C DRIVER
###############################################################################

section "I2C DRIVER"

if [ -d /sys/class/i2c-adapter ]; then

    pass "I2C adapter subsystem available."

else

    fail "I2C adapter subsystem unavailable."

fi

if ls /dev/i2c-* >/dev/null 2>&1; then

    log "I2C device nodes:"
    ls -l /dev/i2c-* |
        tee -a "${LOG_FILE}"

    pass "I2C device nodes detected."

else

    warn "No I2C device nodes detected."

fi

if command -v i2cdetect >/dev/null 2>&1; then

    log ""
    log "I2C controllers:"
    i2cdetect -l |
        tee -a "${LOG_FILE}"

else

    warn "i2cdetect is not installed."

fi

###############################################################################
# SPI DRIVER
###############################################################################

section "SPI DRIVER"

if [ -d /sys/class/spidev ]; then

    pass "SPI spidev subsystem available."

else

    warn "SPI spidev sysfs interface unavailable."

fi

if ls /dev/spidev* >/dev/null 2>&1; then

    log "SPI devices:"
    ls -l /dev/spidev* |
        tee -a "${LOG_FILE}"

    pass "SPI device nodes detected."

else

    warn "No SPI device nodes detected."

fi

###############################################################################
# UART DRIVER
###############################################################################

section "UART DRIVER"

UART_FOUND=0

for DEVICE in \
    /dev/ttyS* \
    /dev/ttyO* \
    /dev/ttyAMA* \
    /dev/ttyUSB* \
    /dev/ttyACM*
do

    if [ -e "${DEVICE}" ]; then

        log "UART device: ${DEVICE}"
        UART_FOUND=1

    fi

done

if [ "${UART_FOUND}" -eq 1 ]; then

    pass "UART/serial device nodes detected."

else

    warn "No UART/serial device nodes detected."

fi

log ""
log "Serial drivers:"

find /sys/bus/serial/drivers \
    -mindepth 1 \
    -maxdepth 1 \
    -type d \
    -printf "%f\n" \
    2>/dev/null |
    sort |
    tee -a "${LOG_FILE}" || true

###############################################################################
# PWM DRIVER
###############################################################################

section "PWM DRIVER"

if [ -d /sys/class/pwm ]; then

    PWM_FOUND=0

    for PWMCHIP in /sys/class/pwm/pwmchip*; do

        if [ -d "${PWMCHIP}" ]; then

            PWM_FOUND=1

            log "PWM controller: ${PWMCHIP}"

            if [ -f "${PWMCHIP}/npwm" ]; then
                log "Channels: $(cat "${PWMCHIP}/npwm")"
            fi

            if [ -L "${PWMCHIP}/device/driver" ]; then

                log "Driver:"
                readlink -f "${PWMCHIP}/device/driver" |
                    tee -a "${LOG_FILE}"

            fi

        fi

    done

    if [ "${PWM_FOUND}" -eq 1 ]; then

        pass "PWM controllers detected."

    else

        warn "No PWM controllers detected."

    fi

else

    fail "PWM subsystem unavailable."

fi

###############################################################################
# CAN DRIVER
###############################################################################

section "CAN DRIVER"

if command -v ip >/dev/null 2>&1; then

    CAN_INTERFACES="$(
        ip -o link show 2>/dev/null |
        grep -E "can[0-9]+"
    )"

    if [ -n "${CAN_INTERFACES}" ]; then

        echo "${CAN_INTERFACES}" |
            tee -a "${LOG_FILE}"

        pass "CAN network interface detected."

        log ""
        log "CAN interface details:"

        ip -details link show type can 2>/dev/null |
            tee -a "${LOG_FILE}" || true

    else

        warn "No CAN network interface detected."

    fi

else

    warn "ip command unavailable."

fi

###############################################################################
# ETHERNET DRIVER
###############################################################################

section "ETHERNET DRIVER"

ETH_FOUND=0

for IFACE in /sys/class/net/*; do

    NAME="$(basename "${IFACE}")"

    if [ "${NAME}" = "lo" ]; then
        continue
    fi

    if [ -e "${IFACE}/device" ]; then

        ETH_FOUND=1

        log "Interface: ${NAME}"

        if [ -f "${IFACE}/address" ]; then
            log "MAC: $(cat "${IFACE}/address")"
        fi

        if [ -f "${IFACE}/operstate" ]; then
            log "State: $(cat "${IFACE}/operstate")"
        fi

        if [ -L "${IFACE}/device/driver" ]; then

            log "Driver:"
            readlink -f "${IFACE}/device/driver" |
                tee -a "${LOG_FILE}"

        fi

    fi

done

if [ "${ETH_FOUND}" -eq 1 ]; then

    pass "Ethernet driver/interface detected."

else

    warn "No physical Ethernet interface detected."

fi

###############################################################################
# USB DRIVER
###############################################################################

section "USB DRIVER"

if [ -d /sys/bus/usb ]; then

    pass "USB subsystem available."

else

    fail "USB subsystem unavailable."

fi

if command -v lsusb >/dev/null 2>&1; then

    log "USB devices:"
    lsusb |
        tee -a "${LOG_FILE}"

else

    warn "lsusb is not installed."

fi

###############################################################################
# MMC / EMMC DRIVER
###############################################################################

section "MMC / EMMC DRIVER"

if [ -d /sys/class/mmc_host ]; then

    MMC_FOUND=0

    for MMC in /sys/class/mmc_host/*; do

        if [ -d "${MMC}" ]; then

            MMC_FOUND=1

            log "MMC host: $(basename "${MMC}")"

            if [ -L "${MMC}/device/driver" ]; then

                log "Driver:"
                readlink -f "${MMC}/device/driver" |
                    tee -a "${LOG_FILE}"

            fi

        fi

    done

    if [ "${MMC_FOUND}" -eq 1 ]; then
        pass "MMC host controller detected."
    else
        warn "No MMC host controller detected."
    fi

else

    warn "MMC subsystem unavailable."

fi

###############################################################################
# DRIVER MODULES
###############################################################################

section "LOADED DRIVER MODULES"

if command -v lsmod >/dev/null 2>&1; then

    log "Selected peripheral-related modules:"

    lsmod |
        grep -Ei \
        "gpio|i2c|spi|serial|uart|pwm|can|usb|mmc|"
        "ethernet|phy|spidev|8250|omap" |
        tee -a "${LOG_FILE}" || true

    log ""
    log "Total loaded modules:"
    lsmod | tail -n +2 | wc -l |
        tee -a "${LOG_FILE}"

else

    warn "lsmod unavailable."

fi

###############################################################################
# PLATFORM DRIVER STATUS
###############################################################################

section "PLATFORM DRIVER STATUS"

if [ -d /sys/bus/platform/drivers ]; then

    log "Relevant platform drivers:"

    find /sys/bus/platform/drivers \
        -mindepth 1 \
        -maxdepth 1 \
        -type d \
        -printf "%f\n" \
        2>/dev/null |
        grep -Ei \
        "gpio|i2c|spi|serial|uart|pwm|can|usb|"
        "ethernet|emmc|mmc|omap|ti" |
        sort |
        tee -a "${LOG_FILE}" || true

    pass "Platform driver information collected."

else

    warn "Platform driver information unavailable."

fi

###############################################################################
# DRIVER BINDINGS
###############################################################################

section "DRIVER BINDINGS"

log "I2C drivers:"
find /sys/bus/i2c/drivers \
    -mindepth 1 \
    -maxdepth 1 \
    -type d \
    -printf "%f\n" \
    2>/dev/null |
    sort |
    tee -a "${LOG_FILE}" || true

log ""
log "SPI drivers:"
find /sys/bus/spi/drivers \
    -mindepth 1 \
    -maxdepth 1 \
    -type d \
    -printf "%f\n" \
    2>/dev/null |
    sort |
    tee -a "${LOG_FILE}" || true

log ""
log "USB drivers:"
find /sys/bus/usb/drivers \
    -mindepth 1 \
    -maxdepth 1 \
    -type d \
    -printf "%f\n" \
    2>/dev/null |
    sort |
    tee -a "${LOG_FILE}" || true

###############################################################################
# INTERRUPT STATUS
###############################################################################

section "INTERRUPT STATUS"

if [ -f /proc/interrupts ]; then

    log "Peripheral interrupts:"

    grep -Ei \
        "gpio|i2c|spi|uart|serial|pwm|can|usb|mmc|eth|"
        "eqep|dma" \
        /proc/interrupts |
        tee -a "${LOG_FILE}" || true

else

    warn "/proc/interrupts unavailable."

fi

###############################################################################
# KERNEL DRIVER ERRORS
###############################################################################

section "KERNEL DRIVER ERRORS"

DRIVER_ERRORS="$(
    dmesg 2>/dev/null |
    grep -iE \
    "driver.*error|driver.*fail|probe.*fail|probe.*error|"
    "failed to probe|unable to.*driver|"
    "gpio.*error|i2c.*error|spi.*error|uart.*error|"
    "pwm.*error|can.*error|ethernet.*error|"
    "usb.*error|mmc.*error" |
    tail -100
)"

if [ -n "${DRIVER_ERRORS}" ]; then

    echo "${DRIVER_ERRORS}" |
        tee -a "${LOG_FILE}"

    warn "Potential driver errors found in kernel log."

else

    pass "No obvious peripheral driver errors found."

fi

###############################################################################
# FAILED SYSTEMD UNITS
###############################################################################

section "FAILED SYSTEM SERVICES"

if command -v systemctl >/dev/null 2>&1; then

    FAILED_SERVICES="$(
        systemctl --failed --no-legend 2>/dev/null
    )"

    if [ -n "${FAILED_SERVICES}" ]; then

        echo "${FAILED_SERVICES}" |
            tee -a "${LOG_FILE}"

        warn "Failed system services detected."

    else

        pass "No failed system services detected."

    fi

else

    warn "systemctl unavailable."

fi

###############################################################################
# FINAL SUMMARY
###############################################################################

section "DRIVER STATUS SUMMARY"

echo "PASS : ${PASS}" | tee -a "${LOG_FILE}"
echo "FAIL : ${FAIL}" | tee -a "${LOG_FILE}"
echo "WARN : ${WARN}" | tee -a "${LOG_FILE}"

log ""
log "Driver status check completed."

log ""
log "Results saved to:"
log "${LOG_FILE}"

log ""
log "============================================================"
log " END OF DRIVER STATUS"
log "============================================================"

###############################################################################
# Exit Status
###############################################################################

if [ "${FAIL}" -eq 0 ]; then
    exit 0
else
    exit 1
fi
