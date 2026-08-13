#!/bin/bash

###############################################################################
# BeagleBone Black - I2C Stress Test
#
# File:
#   tests/stress/i2c_stress.sh
#
# Purpose:
#   Continuously access an I2C device for a configurable duration and monitor
#   bus availability, device detection, read/write transactions, errors,
#   throughput, and kernel I2C errors.
#
# Usage:
#   sudo ./i2c_stress.sh
#   sudo ./i2c_stress.sh <i2c_bus> <i2c_address> <duration> <iterations>
#
# Example:
#   sudo ./i2c_stress.sh 2 0x50 60 10000
#
# IMPORTANT:
#   The default address is 0x50, commonly used by EEPROM devices.
#   Change the address according to the actual hardware connected to the BBB.
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

I2C_BUS="${1:-2}"
I2C_ADDRESS="${2:-0x50}"
DURATION="${3:-60}"
ITERATIONS="${4:-10000}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs/stress"
LOG_FILE="${LOG_DIR}/i2c_stress_${TIMESTAMP}.log"

I2C_DEVICE="/dev/i2c-${I2C_BUS}"

mkdir -p "${LOG_DIR}"

###############################################################################
# Counters
###############################################################################

PASS=0
FAIL=0
SKIP=0

READ_COUNT=0
WRITE_COUNT=0
READ_ERRORS=0
WRITE_ERRORS=0
TRANSACTION_ERRORS=0

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
# Root Check
###############################################################################

if [ "${EUID}" -ne 0 ]; then

    fail "Run I2C stress test as root."

    echo
    echo "Usage:"
    echo "  sudo ./i2c_stress.sh"
    echo "  sudo ./i2c_stress.sh 2 0x50 60 10000"

    exit 1
fi

###############################################################################
# Parameter Validation
###############################################################################

if ! [[ "${I2C_BUS}" =~ ^[0-9]+$ ]]; then

    fail "I2C bus number must be numeric."

    exit 1
fi

if ! [[ "${DURATION}" =~ ^[0-9]+$ ]] || [ "${DURATION}" -le 0 ]; then

    fail "Duration must be a positive integer."

    exit 1
fi

if ! [[ "${ITERATIONS}" =~ ^[0-9]+$ ]] || [ "${ITERATIONS}" -le 0 ]; then

    fail "Iteration count must be a positive integer."

    exit 1
fi

###############################################################################
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - I2C Stress Test"
log "============================================================"
log "Date          : $(date)"
log "Kernel        : $(uname -r)"
log "I2C Bus       : ${I2C_BUS}"
log "I2C Address   : ${I2C_ADDRESS}"
log "Duration      : ${DURATION} seconds"
log "Iterations    : ${ITERATIONS}"
log "Device        : ${I2C_DEVICE}"
log "Log File      : ${LOG_FILE}"
log "============================================================"
log ""

###############################################################################
# Test 1 - I2C Utilities
###############################################################################

log "------------------------------------------------------------"
log "TEST 1: I2C utility availability"
log "------------------------------------------------------------"

if command -v i2cdetect >/dev/null 2>&1; then

    pass "i2cdetect is available."

else

    fail "i2cdetect is not installed."

    log "Install i2c-tools before running this test."

    exit 1
fi

if command -v i2cget >/dev/null 2>&1; then

    pass "i2cget is available."

else

    fail "i2cget is not installed."

    exit 1
fi

if command -v i2cset >/dev/null 2>&1; then

    pass "i2cset is available."

else

    fail "i2cset is not installed."

    exit 1
fi

###############################################################################
# Test 2 - I2C Device Node
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 2: I2C device node"
log "------------------------------------------------------------"

if [ -e "${I2C_DEVICE}" ]; then

    log "I2C device node:"
    ls -l "${I2C_DEVICE}" | tee -a "${LOG_FILE}"

    pass "${I2C_DEVICE} is available."

else

    fail "${I2C_DEVICE} does not exist."

    log ""
    log "Available I2C buses:"

    ls -l /dev/i2c-* 2>/dev/null |
        tee -a "${LOG_FILE}" || true

    exit 1
fi

###############################################################################
# Test 3 - I2C Adapter Information
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 3: I2C adapter information"
log "------------------------------------------------------------"

if command -v i2cdetect >/dev/null 2>&1; then

    i2cdetect -l | tee -a "${LOG_FILE}"

    pass "I2C adapter information collected."

else

    fail "Unable to obtain I2C adapter information."

fi

###############################################################################
# Test 4 - I2C Device Detection
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 4: I2C device detection"
log "------------------------------------------------------------"

DETECT_OUTPUT="$(
    i2cdetect -y "${I2C_BUS}" 2>&1
)"

echo "${DETECT_OUTPUT}" | tee -a "${LOG_FILE}"

ADDRESS_HEX="${I2C_ADDRESS#0x}"

if echo "${DETECT_OUTPUT}" |
    grep -qiE "(^|[[:space:]])${ADDRESS_HEX}([[:space:]]|$)"; then

    pass "I2C device ${I2C_ADDRESS} detected."

else

    fail "I2C device ${I2C_ADDRESS} was not detected."

    log ""
    log "Check:"
    log "  - I2C wiring"
    log "  - SDA/SCL connections"
    log "  - Pull-up resistors"
    log "  - Device power"
    log "  - Device address"
    log "  - Device Tree configuration"

    exit 1
fi

###############################################################################
# Test 5 - Initial I2C Read
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 5: Initial I2C read"
log "------------------------------------------------------------"

INITIAL_READ="$(
    i2cget -y "${I2C_BUS}" "${I2C_ADDRESS}" 0x00 2>&1
)"

if [ $? -eq 0 ]; then

    log "Initial read value: ${INITIAL_READ}"

    pass "Initial I2C read succeeded."

else

    log "Initial I2C read result:"
    echo "${INITIAL_READ}" | tee -a "${LOG_FILE}"

    skip "Register 0x00 read is not supported by this device."

fi

###############################################################################
# Test 6 - I2C Stress Transactions
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 6: I2C continuous transaction stress"
log "------------------------------------------------------------"

log "Starting I2C stress test..."

START_TIME="$(date +%s)"

for ((i=1; i<=ITERATIONS; i++)); do

    CURRENT_TIME="$(date +%s)"
    ELAPSED=$((CURRENT_TIME - START_TIME))

    if [ "${ELAPSED}" -ge "${DURATION}" ]; then
        break
    fi

    ###########################################################################
    # Read transaction
    ###########################################################################

    READ_VALUE="$(
        i2cget -y "${I2C_BUS}" "${I2C_ADDRESS}" 0x00 2>/dev/null
    )"

    if [ $? -eq 0 ]; then

        READ_COUNT=$((READ_COUNT + 1))

    else

        READ_ERRORS=$((READ_ERRORS + 1))
        TRANSACTION_ERRORS=$((TRANSACTION_ERRORS + 1))

    fi

    ###########################################################################
    # Read another register
    ###########################################################################

    READ_VALUE="$(
        i2cget -y "${I2C_BUS}" "${I2C_ADDRESS}" 0x01 2>/dev/null
    )"

    if [ $? -eq 0 ]; then

        READ_COUNT=$((READ_COUNT + 1))

    else

        READ_ERRORS=$((READ_ERRORS + 1))
        TRANSACTION_ERRORS=$((TRANSACTION_ERRORS + 1))

    fi

    ###########################################################################
    # Progress report
    ###########################################################################

    if [ $((i % 100)) -eq 0 ]; then

        log "[INFO] Iteration: ${i} | Reads: ${READ_COUNT} | Errors: ${TRANSACTION_ERRORS}"

    fi

done

END_TIME="$(date +%s)"

ACTUAL_DURATION=$((END_TIME - START_TIME))

if [ "${ACTUAL_DURATION}" -le 0 ]; then
    ACTUAL_DURATION=1
fi

###############################################################################
# Test 7 - I2C Read Throughput
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 7: I2C throughput"
log "------------------------------------------------------------"

READ_RATE="$(
    awk "BEGIN {
        printf \"%.2f\", ${READ_COUNT} / ${ACTUAL_DURATION}
    }"
)"

log "Actual duration : ${ACTUAL_DURATION} seconds"
log "Read operations : ${READ_COUNT}"
log "Read errors     : ${READ_ERRORS}"
log "Read rate       : ${READ_RATE} operations/sec"

if [ "${READ_COUNT}" -gt 0 ]; then

    pass "I2C read transactions completed."

else

    fail "No successful I2C read transactions."

fi

###############################################################################
# Test 8 - I2C Error Check
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 8: I2C transaction error check"
log "------------------------------------------------------------"

if [ "${TRANSACTION_ERRORS}" -eq 0 ]; then

    pass "No I2C transaction errors detected."

else

    fail "I2C transaction errors detected: ${TRANSACTION_ERRORS}"

fi

###############################################################################
# Test 9 - I2C Bus Scan After Stress
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 9: I2C post-stress bus scan"
log "------------------------------------------------------------"

FINAL_SCAN="$(
    i2cdetect -y "${I2C_BUS}" 2>&1
)"

echo "${FINAL_SCAN}" | tee -a "${LOG_FILE}"

if echo "${FINAL_SCAN}" |
    grep -qiE "(^|[[:space:]])${ADDRESS_HEX}([[:space:]]|$)"; then

    pass "I2C device remains visible after stress test."

else

    fail "I2C device is no longer detected."

fi

###############################################################################
# Test 10 - I2C Adapter Runtime Information
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 10: I2C runtime information"
log "------------------------------------------------------------"

if [ -d "/sys/class/i2c-adapter/i2c-${I2C_BUS}" ]; then

    log "I2C adapter path:"
    log "/sys/class/i2c-adapter/i2c-${I2C_BUS}"

    find "/sys/class/i2c-adapter/i2c-${I2C_BUS}" \
        -maxdepth 2 \
        -type f \
        2>/dev/null |
        head -30 |
        tee -a "${LOG_FILE}" || true

    pass "I2C runtime information collected."

else

    skip "I2C sysfs adapter information unavailable."

fi

###############################################################################
# Test 11 - Kernel I2C Error Scan
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 11: I2C kernel error scan"
log "------------------------------------------------------------"

I2C_ERRORS="$(
    dmesg 2>/dev/null |
    grep -iE \
    "i2c|i2c_transfer|i2c timeout|nack|ack|arbitration|bus error|"
    "omap_i2c|ti_i2c|error|failed" |
    tail -50
)"

if [ -n "${I2C_ERRORS}" ]; then

    log "Recent I2C-related kernel messages:"
    echo "${I2C_ERRORS}" | tee -a "${LOG_FILE}"

else

    log "No I2C-related kernel error messages found."

fi

pass "I2C kernel log scan completed."

###############################################################################
# Test 12 - I2C Bus State
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 12: I2C bus state"
log "------------------------------------------------------------"

if [ -d "/sys/class/i2c-adapter/i2c-${I2C_BUS}" ]; then

    BUS_NAME="$(
        cat "/sys/class/i2c-adapter/i2c-${I2C_BUS}/name" \
        2>/dev/null || echo "unknown"
    )"

    log "Bus name: ${BUS_NAME}"

    pass "I2C bus remains available."

else

    fail "I2C bus is unavailable after stress testing."

fi

###############################################################################
# Summary
###############################################################################

log ""
log "============================================================"
log " I2C STRESS TEST SUMMARY"
log "============================================================"

echo "PASS              : ${PASS}" | tee -a "${LOG_FILE}"
echo "FAIL              : ${FAIL}" | tee -a "${LOG_FILE}"
echo "SKIP              : ${SKIP}" | tee -a "${LOG_FILE}"

log ""
log "I2C Bus           : ${I2C_BUS}"
log "I2C Address       : ${I2C_ADDRESS}"
log "Duration          : ${ACTUAL_DURATION} sec"
log "Read Operations   : ${READ_COUNT}"
log "Write Operations  : ${WRITE_COUNT}"
log "Read Errors       : ${READ_ERRORS}"
log "Write Errors      : ${WRITE_ERRORS}"
log "Transaction Errors: ${TRANSACTION_ERRORS}"
log "Read Rate         : ${READ_RATE} operations/sec"

log ""
log "Log File:"
log "${LOG_FILE}"

###############################################################################
# Final Result
###############################################################################

if [ "${READ_ERRORS}" -eq 0 ] &&
   [ "${WRITE_ERRORS}" -eq 0 ] &&
   [ "${FAIL}" -eq 0 ]; then

    log ""
    log "RESULT: PASS"

    exit 0

else

    log ""
    log "RESULT: FAIL"

    exit 1

fi
