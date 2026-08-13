#!/bin/bash

###############################################################################
# BeagleBone Black - GPIO Functional Test
#
# File:
#   tests/functional/test_gpio.sh
#
# Purpose:
#   Verify GPIO subsystem availability and basic GPIO input/output operation.
#
# Tests:
#   1. GPIO subsystem
#   2. GPIO chips
#   3. GPIO character-device interface
#   4. GPIO input
#   5. GPIO output
#
# Usage:
#   sudo ./tests/functional/test_gpio.sh
#   sudo ./tests/functional/test_gpio.sh <gpiochip> <line>
#
# Example:
#   sudo ./tests/functional/test_gpio.sh gpiochip0 20
#
# IMPORTANT:
#   Only use a GPIO line that is physically safe to control.
#   Do not blindly toggle BeagleBone Black pins connected to other hardware.
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

GPIOCHIP="${1:-}"
GPIO_LINE="${2:-}"

TEST_NAME="GPIO Functional Test"
TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

LOG_DIR="${PROJECT_ROOT}/logs/functional"
LOG_FILE="${LOG_DIR}/gpio_${TIMESTAMP}.log"

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
# Logging
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
    fail "Script must be run as root."
    echo
    echo "Use:"
    echo "  sudo ./tests/functional/test_gpio.sh"
    exit 1
fi

###############################################################################
# Header
###############################################################################

log "============================================================"
log " ${TEST_NAME}"
log "============================================================"
log "Date       : $(date)"
log "Kernel     : $(uname -r)"
log "Board      : $(tr '\0' '\n' < /proc/device-tree/model 2>/dev/null || echo Unknown)"
log "Log File   : ${LOG_FILE}"
log "============================================================"
log ""

###############################################################################
# Test 1 - GPIO Subsystem
###############################################################################

log "------------------------------------------------------------"
log "TEST 1: GPIO subsystem"
log "------------------------------------------------------------"

if [ -d /sys/class/gpio ] || [ -d /sys/bus/gpio ]; then
    pass "GPIO subsystem is available."
else
    fail "GPIO subsystem is not available."
fi

###############################################################################
# Test 2 - GPIO Chips
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 2: GPIO chips"
log "------------------------------------------------------------"

if [ -d /sys/class/gpiochip ]; then

    GPIO_CHIPS=$(find /sys/class/gpiochip \
        -maxdepth 1 \
        -type l \
        2>/dev/null | wc -l)

    if [ "${GPIO_CHIPS}" -gt 0 ]; then
        pass "Detected ${GPIO_CHIPS} GPIO chip(s)."

        log ""
        log "Available GPIO chips:"
        find /sys/class/gpiochip \
            -maxdepth 1 \
            -type l \
            -print 2>/dev/null | tee -a "${LOG_FILE}"
    else
        fail "No GPIO chips detected."
    fi

else
    fail "/sys/class/gpiochip is not available."
fi

###############################################################################
# Test 3 - GPIO Character Device
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 3: GPIO character-device interface"
log "------------------------------------------------------------"

if command -v gpiodetect >/dev/null 2>&1; then

    log "gpiodetect output:"
    gpiodetect 2>&1 | tee -a "${LOG_FILE}"

    if gpiodetect >/dev/null 2>&1; then
        pass "GPIO character-device interface is working."
    else
        fail "gpiodetect failed."
    fi

else
    skip "gpiodetect is not installed."
    log "Install libgpiod tools if required."
fi

###############################################################################
# Test 4 - GPIO Information
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 4: GPIO line information"
log "------------------------------------------------------------"

if command -v gpioinfo >/dev/null 2>&1; then

    log "gpioinfo output:"
    gpioinfo 2>&1 | tee -a "${LOG_FILE}"

    pass "GPIO line information successfully retrieved."

else

    skip "gpioinfo is not installed."

fi

###############################################################################
# Test 5 - User Selected GPIO
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 5: User-selected GPIO"
log "------------------------------------------------------------"

if [ -z "${GPIOCHIP}" ] || [ -z "${GPIO_LINE}" ]; then

    skip "No GPIO chip/line supplied for active GPIO test."

    log ""
    log "To perform an actual GPIO line test:"
    log ""
    log "  sudo ./tests/functional/test_gpio.sh gpiochip0 20"
    log ""
    log "Use gpioinfo to identify a safe GPIO line."

else

    log "GPIO chip : ${GPIOCHIP}"
    log "GPIO line : ${GPIO_LINE}"

    if ! command -v gpioset >/dev/null 2>&1; then

        skip "gpioset is not installed."

    else

        log ""
        log "Setting GPIO HIGH..."

        if gpioset "${GPIOCHIP}" "${GPIO_LINE}=1" 2>&1 | \
            tee -a "${LOG_FILE}"; then

            pass "GPIO HIGH operation completed."

        else

            fail "GPIO HIGH operation failed."

        fi

        sleep 1

        log ""
        log "Setting GPIO LOW..."

        if gpioset "${GPIOCHIP}" "${GPIO_LINE}=0" 2>&1 | \
            tee -a "${LOG_FILE}"; then

            pass "GPIO LOW operation completed."

        else

            fail "GPIO LOW operation failed."

        fi

    fi
fi

###############################################################################
# Test 6 - GPIO Kernel Messages
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 6: GPIO kernel messages"
log "------------------------------------------------------------"

log "Recent GPIO-related kernel messages:"

dmesg 2>/dev/null | \
    grep -iE "gpio|gpiolib" | \
    tail -30 | \
    tee -a "${LOG_FILE}" || true

pass "GPIO kernel-log inspection completed."

###############################################################################
# Summary
###############################################################################

log ""
log "============================================================"
log " GPIO TEST SUMMARY"
log "============================================================"

echo -e "${GREEN}PASS : ${PASS}${NC}" | tee -a "${LOG_FILE}"
echo -e "${RED}FAIL : ${FAIL}${NC}" | tee -a "${LOG_FILE}"
echo -e "${YELLOW}SKIP : ${SKIP}${NC}" | tee -a "${LOG_FILE}"

log ""
log "Log:"
log "${LOG_FILE}"

###############################################################################
# Result
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
