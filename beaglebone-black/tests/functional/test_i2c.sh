#!/bin/bash

###############################################################################
# BeagleBone Black - I2C Functional Test
#
# File:
#   tests/functional/test_i2c.sh
#
# Purpose:
#   Verify Linux I2C subsystem, adapters, bus scanning and optional
#   read/write communication with a user-selected I2C device.
#
# Tests:
#   1. I2C subsystem availability
#   2. I2C adapter detection
#   3. I2C bus listing
#   4. I2C bus scan
#   5. Optional device read
#   6. Optional register read
#   7. Kernel I2C messages
#
# Usage:
#
#   sudo ./tests/functional/test_i2c.sh
#   sudo ./tests/functional/test_i2c.sh 2
#   sudo ./tests/functional/test_i2c.sh 2 0x48
#   sudo ./tests/functional/test_i2c.sh 2 0x48 0x00
#
# Examples:
#
#   Scan all I2C buses:
#       sudo ./tests/functional/test_i2c.sh
#
#   Scan I2C bus 2:
#       sudo ./tests/functional/test_i2c.sh 2
#
#   Test device 0x48 on bus 2:
#       sudo ./tests/functional/test_i2c.sh 2 0x48
#
#   Read register 0x00 from device 0x48:
#       sudo ./tests/functional/test_i2c.sh 2 0x48 0x00
#
# IMPORTANT:
#   Do not perform write operations on unknown I2C devices.
#   Reading a register may have side effects on some devices.
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

BUS="${1:-}"
DEVICE="${2:-}"
REGISTER="${3:-}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs/functional"
LOG_FILE="${LOG_DIR}/i2c_${TIMESTAMP}.log"

mkdir -p "${LOG_DIR}"

###############################################################################
# Colors
###############################################################################

GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[1;33m"
BLUE="\033[0;34m"
NC="\033[0m"

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
    echo -e "${GREEN}[PASS]${NC} $1" | tee -a "${LOG_FILE}"
    PASS=$((PASS + 1))
}

fail()
{
    echo -e "${RED}[FAIL]${NC} $1" | tee -a "${LOG_FILE}"
    FAIL=$((FAIL + 1))
}

skip()
{
    echo -e "${YELLOW}[SKIP]${NC} $1" | tee -a "${LOG_FILE}"
    SKIP=$((SKIP + 1))
}

###############################################################################
# Root Check
###############################################################################

if [ "${EUID}" -ne 0 ]; then

    fail "This test must be run as root."

    echo
    echo "Use:"
    echo "  sudo ./tests/functional/test_i2c.sh"

    exit 1
fi

###############################################################################
# Required Command Check
###############################################################################

if ! command -v i2cdetect >/dev/null 2>&1; then

    fail "i2cdetect is not installed."

    echo
    echo "Install i2c-tools before running this test."

    exit 1
fi

###############################################################################
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - I2C Functional Test"
log "============================================================"
log "Date       : $(date)"
log "Kernel     : $(uname -r)"
log "Board      : $(tr '\0' '\n' < /proc/device-tree/model 2>/dev/null || echo Unknown)"
log "Bus        : ${BUS:-ALL}"
log "Device     : ${DEVICE:-NONE}"
log "Register   : ${REGISTER:-NONE}"
log "Log File   : ${LOG_FILE}"
log "============================================================"
log ""

###############################################################################
# Test 1 - I2C Subsystem
###############################################################################

log "------------------------------------------------------------"
log "TEST 1: I2C subsystem"
log "------------------------------------------------------------"

if [ -d /sys/bus/i2c ]; then

    pass "Linux I2C subsystem is available."

else

    fail "Linux I2C subsystem is not available."

    exit 1

fi

###############################################################################
# Test 2 - I2C Adapter Detection
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 2: I2C adapter detection"
log "------------------------------------------------------------"

I2C_ADAPTERS="$(find /sys/class/i2c-adapter \
    -maxdepth 1 \
    -type l \
    2>/dev/null || true)"

if [ -n "${I2C_ADAPTERS}" ]; then

    I2C_COUNT="$(echo "${I2C_ADAPTERS}" | wc -l)"

    pass "Detected ${I2C_COUNT} I2C adapter(s)."

else

    fail "No I2C adapters detected."

    exit 1

fi

###############################################################################
# Test 3 - I2C Bus List
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 3: I2C bus listing"
log "------------------------------------------------------------"

log "Available I2C buses:"

i2cdetect -l 2>&1 | tee -a "${LOG_FILE}"

if i2cdetect -l >/dev/null 2>&1; then

    pass "I2C bus listing completed."

else

    fail "Unable to list I2C buses."

fi

###############################################################################
# Function - Scan Bus
###############################################################################

scan_bus()
{
    I2C_BUS="$1"

    log ""
    log "Scanning I2C bus ${I2C_BUS}..."

    if [ ! -e "/dev/i2c-${I2C_BUS}" ]; then

        fail "/dev/i2c-${I2C_BUS} does not exist."

        return 1
    fi

    log ""
    log "I2C scan result for bus ${I2C_BUS}:"

    if i2cdetect -y "${I2C_BUS}" 2>&1 | \
        tee -a "${LOG_FILE}"; then

        pass "I2C bus ${I2C_BUS} scan completed."

    else

        fail "I2C bus ${I2C_BUS} scan failed."

        return 1
    fi
}

###############################################################################
# Test 4 - Bus Scan
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 4: I2C bus scan"
log "------------------------------------------------------------"

if [ -n "${BUS}" ]; then

    if [[ "${BUS}" =~ ^[0-9]+$ ]]; then

        scan_bus "${BUS}"

    else

        fail "Invalid I2C bus number: ${BUS}"

    fi

else

    log "No specific bus supplied."
    log "Scanning all detected I2C buses."

    while read -r BUS_LINE; do

        [ -n "${BUS_LINE}" ] || continue

        BUS_NUM="$(echo "${BUS_LINE}" | \
            sed -n 's/^i2c-\([0-9]*\).*/\1/p')"

        if [ -n "${BUS_NUM}" ]; then

            scan_bus "${BUS_NUM}"

        fi

    done < <(i2cdetect -l 2>/dev/null)

fi

###############################################################################
# Test 5 - Device Selection
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 5: I2C device verification"
log "------------------------------------------------------------"

if [ -z "${DEVICE}" ]; then

    skip "No I2C device address supplied."

    log ""
    log "To verify a specific device:"
    log ""
    log "  sudo ./tests/functional/test_i2c.sh 2 0x48"

else

    if [ -z "${BUS}" ]; then

        fail "Bus number is required when testing a specific device."

    else

        DEVICE_HEX="${DEVICE}"

        if [[ "${DEVICE_HEX}" != 0x* ]]; then
            DEVICE_HEX="0x${DEVICE_HEX}"
        fi

        log "Checking device ${DEVICE_HEX} on bus ${BUS}..."

        if i2cdetect -y "${BUS}" 2>/dev/null | \
            grep -qiE "(^|[[:space:]])${DEVICE_HEX#0x}([[:space:]]|$)"; then

            pass "I2C device ${DEVICE_HEX} detected on bus ${BUS}."

        else

            skip "Device ${DEVICE_HEX} was not detected on bus ${BUS}."

        fi

    fi
fi

###############################################################################
# Test 6 - Optional Register Read
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 6: I2C register read"
log "------------------------------------------------------------"

if [ -z "${BUS}" ] || [ -z "${DEVICE}" ] || [ -z "${REGISTER}" ]; then

    skip "Bus, device and register were not all supplied."

    log ""
    log "Example:"
    log "  sudo ./tests/functional/test_i2c.sh 2 0x48 0x00"

else

    DEVICE_VALUE="${DEVICE#0x}"
    REGISTER_VALUE="${REGISTER#0x}"

    log "Bus      : ${BUS}"
    log "Device   : 0x${DEVICE_VALUE}"
    log "Register : 0x${REGISTER_VALUE}"

    if command -v i2cget >/dev/null 2>&1; then

        log ""
        log "Reading register..."

        if VALUE="$(i2cget \
            -y \
            "${BUS}" \
            "0x${DEVICE_VALUE}" \
            "0x${REGISTER_VALUE}" \
            2>&1)"; then

            log "Register value: ${VALUE}"

            pass "I2C register read successful."

        else

            fail "I2C register read failed."

            log "${VALUE}"

        fi

    else

        skip "i2cget is not installed."

    fi

fi

###############################################################################
# Test 7 - I2C Sysfs Devices
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 7: I2C kernel devices"
log "------------------------------------------------------------"

I2C_DEVICES="$(find /sys/bus/i2c/devices \
    -maxdepth 1 \
    -type l \
    2>/dev/null || true)"

if [ -n "${I2C_DEVICES}" ]; then

    log "Registered I2C devices:"

    echo "${I2C_DEVICES}" | tee -a "${LOG_FILE}"

    pass "I2C kernel devices detected."

else

    skip "No I2C slave devices are currently registered."

fi

###############################################################################
# Test 8 - Kernel Logs
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 8: I2C kernel messages"
log "------------------------------------------------------------"

log "Recent I2C-related kernel messages:"

dmesg 2>/dev/null | \
    grep -iE "i2c|omap_i2c|ti_i2c" | \
    tail -50 | \
    tee -a "${LOG_FILE}" || true

pass "I2C kernel-log inspection completed."

###############################################################################
# Summary
###############################################################################

log ""
log "============================================================"
log " I2C TEST SUMMARY"
log "============================================================"

echo -e "${GREEN}PASS : ${PASS}${NC}" | tee -a "${LOG_FILE}"
echo -e "${RED}FAIL : ${FAIL}${NC}" | tee -a "${LOG_FILE}"
echo -e "${YELLOW}SKIP : ${SKIP}${NC}" | tee -a "${LOG_FILE}"

log ""
log "Test log:"
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
