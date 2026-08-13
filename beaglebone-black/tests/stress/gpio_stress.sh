#!/bin/bash

###############################################################################
# BeagleBone Black - GPIO Stress Test
#
# File:
#   tests/stress/gpio_stress.sh
#
# Purpose:
#   Repeatedly toggle GPIO pins and verify GPIO accessibility, state changes,
#   read/write operations, and kernel errors for a configurable duration.
#
# Usage:
#   sudo ./gpio_stress.sh
#   sudo ./gpio_stress.sh <gpio_number> <duration_seconds> <toggle_count>
#
# Example:
#   sudo ./gpio_stress.sh 60 60 10000
#
# IMPORTANT:
#   GPIO numbering depends on the Linux GPIO configuration.
#   Do not connect GPIO pins directly to incompatible voltage sources.
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

GPIO_NUMBER="${1:-60}"
DURATION="${2:-60}"
TOGGLE_COUNT="${3:-10000}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs/stress"
LOG_FILE="${LOG_DIR}/gpio_stress_${TIMESTAMP}.log"

GPIO_SYSFS="/sys/class/gpio/gpio${GPIO_NUMBER}"

mkdir -p "${LOG_DIR}"

###############################################################################
# Counters
###############################################################################

PASS=0
FAIL=0
SKIP=0

TOGGLES=0
READ_ERRORS=0
WRITE_ERRORS=0
STATE_ERRORS=0

###############################################################################
# Logging
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
# Cleanup
###############################################################################

cleanup()
{
    if [ -d "${GPIO_SYSFS}" ]; then
        echo "in" > "${GPIO_SYSFS}/direction" 2>/dev/null || true
    fi
}

trap cleanup EXIT INT TERM

###############################################################################
# Root Check
###############################################################################

if [ "${EUID}" -ne 0 ]; then

    fail "Run GPIO stress test as root."

    echo
    echo "Usage:"
    echo "  sudo ./gpio_stress.sh"
    echo "  sudo ./gpio_stress.sh 60 60 10000"

    exit 1
fi

###############################################################################
# Parameter Validation
###############################################################################

if ! [[ "${GPIO_NUMBER}" =~ ^[0-9]+$ ]]; then
    fail "GPIO number must be numeric."
    exit 1
fi

if ! [[ "${DURATION}" =~ ^[0-9]+$ ]] || [ "${DURATION}" -le 0 ]; then
    fail "Duration must be a positive integer."
    exit 1
fi

if ! [[ "${TOGGLE_COUNT}" =~ ^[0-9]+$ ]] || [ "${TOGGLE_COUNT}" -le 0 ]; then
    fail "Toggle count must be a positive integer."
    exit 1
fi

###############################################################################
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - GPIO Stress Test"
log "============================================================"
log "Date          : $(date)"
log "Kernel        : $(uname -r)"
log "GPIO Number   : ${GPIO_NUMBER}"
log "Duration      : ${DURATION} seconds"
log "Toggle Count  : ${TOGGLE_COUNT}"
log "Log File      : ${LOG_FILE}"
log "============================================================"
log ""

###############################################################################
# Test 1 - GPIO Subsystem
###############################################################################

log "------------------------------------------------------------"
log "TEST 1: GPIO subsystem"
log "------------------------------------------------------------"

if [ -d /sys/class/gpio ] || [ -d /sys/bus/gpio ]; then

    pass "Linux GPIO subsystem is available."

else

    fail "Linux GPIO subsystem is unavailable."

    exit 1
fi

###############################################################################
# Test 2 - GPIO Character Device
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 2: GPIO character device"
log "------------------------------------------------------------"

if ls /dev/gpiochip* >/dev/null 2>&1; then

    log "GPIO character devices:"

    ls -l /dev/gpiochip* | tee -a "${LOG_FILE}"

    pass "GPIO character devices detected."

else

    skip "No GPIO character devices detected."

fi

###############################################################################
# Test 3 - GPIO Export
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 3: GPIO availability"
log "------------------------------------------------------------"

if [ ! -d "${GPIO_SYSFS}" ]; then

    if [ -w /sys/class/gpio/export ]; then

        if echo "${GPIO_NUMBER}" > /sys/class/gpio/export 2>/dev/null; then

            sleep 0.2

            pass "GPIO ${GPIO_NUMBER} exported successfully."

        else

            fail "Unable to export GPIO ${GPIO_NUMBER}."

            exit 1
        fi

    else

        fail "GPIO sysfs export interface is unavailable."

        exit 1
    fi

else

    pass "GPIO ${GPIO_NUMBER} is already exported."

fi

###############################################################################
# Test 4 - GPIO Direction
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 4: GPIO output configuration"
log "------------------------------------------------------------"

if echo "out" > "${GPIO_SYSFS}/direction" 2>/dev/null; then

    pass "GPIO ${GPIO_NUMBER} configured as output."

else

    fail "Unable to configure GPIO as output."

    exit 1
fi

###############################################################################
# Test 5 - Initial GPIO State
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 5: Initial GPIO state"
log "------------------------------------------------------------"

if [ -r "${GPIO_SYSFS}/value" ]; then

    INITIAL_VALUE="$(cat "${GPIO_SYSFS}/value" 2>/dev/null || true)"

    if [ "${INITIAL_VALUE}" = "0" ] || [ "${INITIAL_VALUE}" = "1" ]; then

        log "Initial GPIO value: ${INITIAL_VALUE}"

        pass "Initial GPIO state is valid."

    else

        fail "Invalid initial GPIO value."

    fi

else

    fail "GPIO value interface is not readable."

fi

###############################################################################
# Test 6 - Basic GPIO Toggle
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 6: Basic GPIO toggle"
log "------------------------------------------------------------"

BASIC_FAILURE=0

for VALUE in 0 1 0 1; do

    if ! echo "${VALUE}" > "${GPIO_SYSFS}/value" 2>/dev/null; then

        BASIC_FAILURE=$((BASIC_FAILURE + 1))

        continue
    fi

    sleep 0.01

    READ_VALUE="$(cat "${GPIO_SYSFS}/value" 2>/dev/null || true)"

    if [ "${READ_VALUE}" != "${VALUE}" ]; then

        BASIC_FAILURE=$((BASIC_FAILURE + 1))

        log "[ERROR] Expected ${VALUE}, read ${READ_VALUE}"

    fi

done

if [ "${BASIC_FAILURE}" -eq 0 ]; then

    pass "Basic GPIO read/write toggle test passed."

else

    fail "Basic GPIO toggle test failed."

fi

###############################################################################
# Test 7 - GPIO Stress Loop
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 7: GPIO continuous toggle stress"
log "------------------------------------------------------------"

log "Starting GPIO stress test..."

START_TIME="$(date +%s)"

CURRENT_VALUE=0

while true; do

    CURRENT_TIME="$(date +%s)"
    ELAPSED=$((CURRENT_TIME - START_TIME))

    if [ "${TOGGLES}" -ge "${TOGGLE_COUNT}" ]; then
        break
    fi

    if [ "${ELAPSED}" -ge "${DURATION}" ]; then
        break
    fi

    if [ "${CURRENT_VALUE}" -eq 0 ]; then
        CURRENT_VALUE=1
    else
        CURRENT_VALUE=0
    fi

    if echo "${CURRENT_VALUE}" > "${GPIO_SYSFS}/value" 2>/dev/null; then

        TOGGLES=$((TOGGLES + 1))

    else

        WRITE_ERRORS=$((WRITE_ERRORS + 1))

        continue
    fi

    READ_VALUE="$(cat "${GPIO_SYSFS}/value" 2>/dev/null || true)"

    if [ "${READ_VALUE}" != "${CURRENT_VALUE}" ]; then

        READ_ERRORS=$((READ_ERRORS + 1))

    fi

    if [ $((TOGGLES % 1000)) -eq 0 ]; then

        log "[INFO] Toggles: ${TOGGLES} | Read errors: ${READ_ERRORS} | Write errors: ${WRITE_ERRORS}"

    fi

done

END_TIME="$(date +%s)"

ACTUAL_DURATION=$((END_TIME - START_TIME))

if [ "${ACTUAL_DURATION}" -le 0 ]; then
    ACTUAL_DURATION=1
fi

###############################################################################
# Test 8 - Toggle Rate
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 8: GPIO toggle rate"
log "------------------------------------------------------------"

TOGGLE_RATE="$(
    awk "BEGIN {
        printf \"%.2f\", ${TOGGLES} / ${ACTUAL_DURATION}
    }"
)"

log "Actual duration : ${ACTUAL_DURATION} seconds"
log "Total toggles   : ${TOGGLES}"
log "Toggle rate     : ${TOGGLE_RATE} toggles/sec"

if [ "${TOGGLES}" -gt 0 ]; then

    pass "GPIO toggle rate calculated."

else

    fail "No GPIO toggles completed."

fi

###############################################################################
# Test 9 - GPIO State Verification
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 9: Final GPIO state verification"
log "------------------------------------------------------------"

FINAL_VALUE="$(cat "${GPIO_SYSFS}/value" 2>/dev/null || true)"

if [ "${FINAL_VALUE}" = "0" ] || [ "${FINAL_VALUE}" = "1" ]; then

    log "Final GPIO value: ${FINAL_VALUE}"

    pass "Final GPIO state is valid."

else

    STATE_ERRORS=$((STATE_ERRORS + 1))

    fail "Final GPIO state is invalid."

fi

###############################################################################
# Test 10 - GPIO Kernel Error Scan
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 10: GPIO kernel error scan"
log "------------------------------------------------------------"

GPIO_ERRORS="$(
    dmesg 2>/dev/null |
    grep -iE "gpio|gpiolib|pinctrl|pinmux|gpiochip|error|failed" |
    tail -50
)"

if [ -n "${GPIO_ERRORS}" ]; then

    log "Recent GPIO-related kernel messages:"
    echo "${GPIO_ERRORS}" | tee -a "${LOG_FILE}"

else

    log "No GPIO-related kernel errors found."

fi

pass "GPIO kernel log scan completed."

###############################################################################
# Test 11 - GPIO Controller Information
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 11: GPIO controller information"
log "------------------------------------------------------------"

if [ -f /sys/kernel/debug/gpio ]; then

    cat /sys/kernel/debug/gpio 2>/dev/null |
        tee -a "${LOG_FILE}" || true

    pass "GPIO controller debug information collected."

else

    skip "GPIO debug information is unavailable."

fi

###############################################################################
# Test 12 - Pinctrl Information
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 12: Pin control information"
log "------------------------------------------------------------"

if [ -d /sys/kernel/debug/pinctrl ]; then

    find /sys/kernel/debug/pinctrl \
        -maxdepth 2 \
        -type f \
        -name "pinmux-pins" \
        -exec sh -c 'echo "---- $1 ----"; cat "$1"' _ {} \; \
        2>/dev/null |
        tee -a "${LOG_FILE}" || true

    pass "Pinctrl information collected."

else

    skip "Pinctrl debug filesystem is unavailable."

fi

###############################################################################
# Summary
###############################################################################

log ""
log "============================================================"
log " GPIO STRESS TEST SUMMARY"
log "============================================================"

echo "PASS             : ${PASS}" | tee -a "${LOG_FILE}"
echo "FAIL             : ${FAIL}" | tee -a "${LOG_FILE}"
echo "SKIP             : ${SKIP}" | tee -a "${LOG_FILE}"

log ""
log "GPIO Number      : ${GPIO_NUMBER}"
log "Duration         : ${ACTUAL_DURATION} sec"
log "Requested Toggles: ${TOGGLE_COUNT}"
log "Completed Toggles: ${TOGGLES}"
log "Read Errors      : ${READ_ERRORS}"
log "Write Errors     : ${WRITE_ERRORS}"
log "State Errors     : ${STATE_ERRORS}"
log "Toggle Rate      : ${TOGGLE_RATE} toggles/sec"
log "Final GPIO Value  : ${FINAL_VALUE:-UNKNOWN}"

log ""
log "Log File:"
log "${LOG_FILE}"

###############################################################################
# Final Result
###############################################################################

if [ "${READ_ERRORS}" -eq 0 ] &&
   [ "${WRITE_ERRORS}" -eq 0 ] &&
   [ "${STATE_ERRORS}" -eq 0 ] &&
   [ "${FAIL}" -eq 0 ]; then

    log ""
    log "RESULT: PASS"

    exit 0

else

    log ""
    log "RESULT: FAIL"

    exit 1

fi
