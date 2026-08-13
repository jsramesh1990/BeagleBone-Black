#!/bin/bash

###############################################################################
# BeagleBone Black - Driver Regression Test
#
# File:
#   tests/regression/driver_regression.sh
#
# Purpose:
#   Verify that required Linux peripheral drivers are available, loaded,
#   bound to hardware, and exposed through the expected Linux interfaces.
#
# Usage:
#   sudo ./driver_regression.sh
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs/regression"
LOG_FILE="${LOG_DIR}/driver_regression_${TIMESTAMP}.log"

mkdir -p "${LOG_DIR}"

###############################################################################
# Counters
###############################################################################

PASS=0
FAIL=0
SKIP=0

###############################################################################
# Logging Functions
###############################################################################

log()
{
    echo "$1" | tee -a "${LOG_FILE}"
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

skip()
{
    echo "[SKIP] $1" | tee -a "${LOG_FILE}"
    SKIP=$((SKIP + 1))
}

###############################################################################
# Helper Functions
###############################################################################

check_path()
{
    local path="$1"
    local description="$2"

    if [ -e "${path}" ]; then
        pass "${description}"
        return 0
    else
        fail "${description}"
        return 1
    fi
}

check_command()
{
    local command_name="$1"
    local description="$2"

    if command -v "${command_name}" >/dev/null 2>&1; then
        pass "${description}"
        return 0
    else
        skip "${description}"
        return 1
    fi
}

check_module()
{
    local module="$1"
    local description="$2"

    if lsmod | awk '{print $1}' | grep -qx "${module}"; then

        pass "${description} - module loaded."

        return 0

    elif modinfo "${module}" >/dev/null 2>&1; then

        skip "${description} - module available but not currently loaded."

        return 1

    else

        skip "${description} - module not found."

        return 1

    fi
}

###############################################################################
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - Driver Regression Test"
log "============================================================"
log "Date       : $(date)"
log "Kernel     : $(uname -r)"
log "Architecture: $(uname -m)"
log "Log File   : ${LOG_FILE}"
log "============================================================"
log ""

###############################################################################
# Test 1 - Kernel Driver Infrastructure
###############################################################################

log "------------------------------------------------------------"
log "TEST 1: Kernel driver infrastructure"
log "------------------------------------------------------------"

if [ -d /sys/bus ] && [ -d /sys/class ] && [ -d /sys/module ]; then

    pass "Linux kernel driver infrastructure is available."

else

    fail "Required kernel driver infrastructure is missing."

    exit 1
fi

###############################################################################
# Test 2 - Device Tree Driver Binding
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 2: Device Tree driver binding"
log "------------------------------------------------------------"

if [ -d /sys/bus/platform/drivers ]; then

    PLATFORM_DRIVER_COUNT="$(
        find /sys/bus/platform/drivers \
        -mindepth 1 -maxdepth 1 -type d 2>/dev/null | wc -l
    )"

    log "Platform drivers registered: ${PLATFORM_DRIVER_COUNT}"

    if [ "${PLATFORM_DRIVER_COUNT}" -gt 0 ]; then
        pass "Platform driver subsystem is populated."
    else
        fail "No platform drivers found."
    fi

else

    fail "Platform driver directory is unavailable."

fi

###############################################################################
# Test 3 - GPIO Driver
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 3: GPIO driver"
log "------------------------------------------------------------"

GPIO_FOUND=0

if [ -d /sys/class/gpio ]; then

    pass "GPIO sysfs interface is available."
    GPIO_FOUND=1

elif [ -d /sys/bus/gpio ]; then

    pass "GPIO kernel subsystem is available."
    GPIO_FOUND=1

else

    fail "GPIO kernel interface is unavailable."

fi

if [ -d /sys/kernel/debug/gpio ]; then

    log ""
    log "GPIO controller information:"

    cat /sys/kernel/debug/gpio 2>/dev/null | \
        tee -a "${LOG_FILE}" || true

else

    skip "GPIO debug information is not available."

fi

###############################################################################
# Test 4 - I2C Driver
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 4: I2C driver"
log "------------------------------------------------------------"

if [ -d /sys/class/i2c-adapter ]; then

    I2C_COUNT="$(
        find /sys/class/i2c-adapter \
        -mindepth 1 -maxdepth 1 -type l 2>/dev/null | wc -l
    )"

    log "Detected I2C adapters: ${I2C_COUNT}"

    if [ "${I2C_COUNT}" -gt 0 ]; then

        pass "I2C driver is active and adapters are registered."

    else

        fail "No I2C adapters detected."

    fi

else

    fail "I2C subsystem is unavailable."

fi

if command -v i2cdetect >/dev/null 2>&1; then

    pass "i2c-tools is installed."

else

    skip "i2cdetect is not installed."

fi

###############################################################################
# Test 5 - SPI Driver
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 5: SPI driver"
log "------------------------------------------------------------"

if [ -d /sys/class/spi_master ]; then

    SPI_MASTER_COUNT="$(
        find /sys/class/spi_master \
        -mindepth 1 -maxdepth 1 -type l 2>/dev/null | wc -l
    )"

    log "Detected SPI masters: ${SPI_MASTER_COUNT}"

    if [ "${SPI_MASTER_COUNT}" -gt 0 ]; then

        pass "SPI master driver is active."

    else

        fail "No SPI master controllers detected."

    fi

else

    fail "SPI master subsystem is unavailable."

fi

if ls /dev/spidev* >/dev/null 2>&1; then

    log "SPI userspace devices:"

    ls -l /dev/spidev* | tee -a "${LOG_FILE}"

    pass "SPI spidev device detected."

else

    skip "No spidev userspace device detected."

fi

###############################################################################
# Test 6 - UART Driver
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 6: UART driver"
log "------------------------------------------------------------"

UART_COUNT="$(
    find /sys/class/tty \
    -maxdepth 1 \
    \( -name 'ttyS*' -o -name 'ttyO*' -o -name 'ttyAMA*' \) \
    2>/dev/null | wc -l
)"

log "Detected UART/serial devices: ${UART_COUNT}"

if [ "${UART_COUNT}" -gt 0 ]; then

    pass "UART/serial drivers are registered."

else

    fail "No UART/serial devices detected."

fi

log ""
log "Serial devices:"

ls -l /dev/ttyS* /dev/ttyO* /dev/ttyAMA* \
    2>/dev/null | tee -a "${LOG_FILE}" || true

###############################################################################
# Test 7 - PWM Driver
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 7: PWM driver"
log "------------------------------------------------------------"

if [ -d /sys/class/pwm ]; then

    PWM_CHIP_COUNT="$(
        find /sys/class/pwm \
        -mindepth 1 -maxdepth 1 \
        -type d -name 'pwmchip*' 2>/dev/null | wc -l
    )"

    log "Detected PWM controllers: ${PWM_CHIP_COUNT}"

    if [ "${PWM_CHIP_COUNT}" -gt 0 ]; then

        pass "PWM drivers/controllers are registered."

    else

        skip "No PWM controllers detected."

    fi

else

    skip "PWM subsystem is unavailable."

fi

###############################################################################
# Test 8 - CAN Driver
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 8: CAN driver"
log "------------------------------------------------------------"

CAN_COUNT="$(
    find /sys/class/net \
    -maxdepth 1 \
    -type l \
    -name 'can*' 2>/dev/null | wc -l
)"

log "Detected CAN network interfaces: ${CAN_COUNT}"

if [ "${CAN_COUNT}" -gt 0 ]; then

    pass "CAN network driver is active."

    log ""
    log "CAN interfaces:"

    ip -details link show type can 2>/dev/null | \
        tee -a "${LOG_FILE}" || true

else

    skip "No CAN network interface detected."

fi

###############################################################################
# Test 9 - ADC Driver
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 9: ADC driver"
log "------------------------------------------------------------"

IIO_COUNT=0

if [ -d /sys/bus/iio/devices ]; then

    IIO_COUNT="$(
        find /sys/bus/iio/devices \
        -maxdepth 1 \
        -type d \
        -name 'iio:device*' 2>/dev/null | wc -l
    )"

    log "Detected IIO devices: ${IIO_COUNT}"

    if [ "${IIO_COUNT}" -gt 0 ]; then

        pass "IIO/ADC driver subsystem is active."

        log ""
        log "IIO devices:"

        ls -l /sys/bus/iio/devices/ | \
            tee -a "${LOG_FILE}" || true

    else

        skip "No IIO/ADC devices detected."

    fi

else

    skip "IIO subsystem is unavailable."

fi

###############################################################################
# Test 10 - Network Driver
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 10: Ethernet/network driver"
log "------------------------------------------------------------"

NETWORK_COUNT="$(
    find /sys/class/net \
    -mindepth 1 \
    -maxdepth 1 \
    -type l 2>/dev/null | wc -l
)"

log "Network interfaces: ${NETWORK_COUNT}"

if [ "${NETWORK_COUNT}" -gt 0 ]; then

    pass "Network driver subsystem is active."

    log ""
    log "Network interfaces:"

    ip -br link 2>/dev/null | tee -a "${LOG_FILE}" || true

else

    fail "No network interfaces detected."

fi

###############################################################################
# Test 11 - USB Driver
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 11: USB driver"
log "------------------------------------------------------------"

if [ -d /sys/bus/usb/devices ]; then

    USB_DEVICE_COUNT="$(
        find /sys/bus/usb/devices \
        -mindepth 1 \
        -maxdepth 1 \
        -type l 2>/dev/null | wc -l
    )"

    log "USB devices/controllers: ${USB_DEVICE_COUNT}"

    if [ "${USB_DEVICE_COUNT}" -gt 0 ]; then

        pass "USB driver subsystem is active."

    else

        skip "No USB devices detected."

    fi

else

    fail "USB subsystem is unavailable."

fi

###############################################################################
# Test 12 - Driver Modules
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 12: Important kernel modules"
log "------------------------------------------------------------"

if command -v lsmod >/dev/null 2>&1; then

    log "Loaded modules related to BBB peripherals:"

    lsmod | grep -iE \
        "gpio|i2c|spi|uart|serial|pwm|can|adc|ti|omap|mcspi|8250" \
        | tee -a "${LOG_FILE}" || true

    pass "Kernel module list inspected."

else

    skip "lsmod command is unavailable."

fi

###############################################################################
# Test 13 - Driver Binding
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 13: Driver binding verification"
log "------------------------------------------------------------"

BOUND_COUNT=0
UNBOUND_COUNT=0

while IFS= read -r DRIVER_LINK; do

    if [ -L "${DRIVER_LINK}" ]; then

        BOUND_COUNT=$((BOUND_COUNT + 1))

    fi

done < <(
    find /sys/bus/platform/devices \
        -maxdepth 1 \
        -type l 2>/dev/null
)

log "Platform device entries: ${BOUND_COUNT}"

if [ "${BOUND_COUNT}" -gt 0 ]; then

    pass "Platform devices are registered."

else

    skip "No platform device entries found."

fi

###############################################################################
# Test 14 - Kernel Driver Errors
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 14: Kernel driver error scan"
log "------------------------------------------------------------"

ERROR_COUNT="$(
    dmesg 2>/dev/null |
    grep -iE \
    "probe.*fail|probe failed|driver.*failed|unable to.*driver|error.*driver|"
    "failed to.*register|timeout.*driver" |
    wc -l
)"

log "Potential driver errors found: ${ERROR_COUNT}"

if [ "${ERROR_COUNT}" -eq 0 ]; then

    pass "No obvious driver errors found in kernel log."

else

    fail "Potential driver errors detected."

    dmesg 2>/dev/null |
        grep -iE \
        "probe.*fail|probe failed|driver.*failed|unable to.*driver|error.*driver|"
        "failed to.*register|timeout.*driver" |
        tail -30 |
        tee -a "${LOG_FILE}" || true

fi

###############################################################################
# Test 15 - Device Nodes
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 15: Peripheral device nodes"
log "------------------------------------------------------------"

DEVICE_NODE_COUNT=0

for pattern in \
    "/dev/i2c-*" \
    "/dev/spidev*" \
    "/dev/ttyS*" \
    "/dev/ttyO*" \
    "/dev/gpiochip*" \
    "/dev/pwm*" \
    "/dev/can*"; do

    for node in ${pattern}; do

        if [ -e "${node}" ]; then

            log "Found: ${node}"

            DEVICE_NODE_COUNT=$((DEVICE_NODE_COUNT + 1))

        fi

    done

done

if [ "${DEVICE_NODE_COUNT}" -gt 0 ]; then

    pass "Peripheral device nodes are present."

else

    skip "No expected peripheral device nodes found."

fi

###############################################################################
# Test 16 - Driver Information
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 16: Driver information"
log "------------------------------------------------------------"

if command -v lspci >/dev/null 2>&1; then

    log "PCI devices and drivers:"

    lspci -k 2>/dev/null | \
        tee -a "${LOG_FILE}" || true

else

    skip "lspci is not installed."

fi

###############################################################################
# Test 17 - Loaded Driver Summary
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 17: Loaded driver summary"
log "------------------------------------------------------------"

log "Selected loaded modules:"

lsmod 2>/dev/null |
    grep -iE \
    "gpio|i2c|spi|uart|serial|pwm|can|adc|ti|omap|mcspi|8250|usb" |
    tee -a "${LOG_FILE}" || true

pass "Loaded driver summary generated."

###############################################################################
# Test 18 - Kernel Log Summary
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 18: Kernel driver messages"
log "------------------------------------------------------------"

dmesg 2>/dev/null |
    grep -iE \
    "gpio|i2c|spi|uart|serial|pwm|can|adc|driver|probe" |
    tail -80 |
    tee -a "${LOG_FILE}" || true

pass "Kernel driver messages inspected."

###############################################################################
# Summary
###############################################################################

log ""
log "============================================================"
log " DRIVER REGRESSION SUMMARY"
log "============================================================"

echo "PASS : ${PASS}" | tee -a "${LOG_FILE}"
echo "FAIL : ${FAIL}" | tee -a "${LOG_FILE}"
echo "SKIP : ${SKIP}" | tee -a "${LOG_FILE}"

log ""
log "I2C Adapters       : ${I2C_COUNT:-0}"
log "SPI Masters        : ${SPI_MASTER_COUNT:-0}"
log "UART Devices       : ${UART_COUNT:-0}"
log "PWM Controllers    : ${PWM_CHIP_COUNT:-0}"
log "CAN Interfaces     : ${CAN_COUNT:-0}"
log "IIO/ADC Devices    : ${IIO_COUNT:-0}"
log "Network Interfaces  : ${NETWORK_COUNT:-0}"
log "USB Devices        : ${USB_DEVICE_COUNT:-0}"
log "Driver Errors      : ${ERROR_COUNT:-0}"
log "Device Nodes       : ${DEVICE_NODE_COUNT}"

log ""
log "Log File:"
log "${LOG_FILE}"

###############################################################################
# Final Result
###############################################################################

if [ "${FAIL}" -eq 0 ]; then

    log ""
    log "RESULT: PASS"

    exit 0

else

    log ""
    log "RESULT: FAIL"

    exit 1

fi
