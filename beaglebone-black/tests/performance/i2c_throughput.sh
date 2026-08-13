#!/bin/bash

###############################################################################
# BeagleBone Black - I2C Throughput Performance Test
#
# File:
#   tests/performance/i2c_throughput.sh
#
# Purpose:
#   Measure I2C transaction throughput, transfer time, and effective
#   data rate for a connected I2C slave device.
#
# Usage:
#   sudo ./i2c_throughput.sh
#   sudo ./i2c_throughput.sh <bus> <address> <register> <count> <bytes>
#
# Example:
#   sudo ./i2c_throughput.sh 2 0x50 0x00 100 16
#
# Parameters:
#   bus       - I2C bus number
#   address   - I2C slave address
#   register  - Register address
#   count     - Number of transactions
#   bytes     - Number of bytes per transaction
#
# Example device:
#   EEPROM at 0x50 on I2C-2
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

I2C_BUS="${1:-2}"
I2C_ADDRESS="${2:-0x50}"
I2C_REGISTER="${3:-0x00}"
TRANSACTION_COUNT="${4:-100}"
BYTES_PER_TRANSFER="${5:-16}"

I2C_DEVICE="/dev/i2c-${I2C_BUS}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs/performance"
LOG_FILE="${LOG_DIR}/i2c_throughput_${TIMESTAMP}.log"

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
# Root Check
###############################################################################

if [ "${EUID}" -ne 0 ]; then

    fail "Run this test as root."

    echo
    echo "Usage:"
    echo "  sudo ./i2c_throughput.sh"

    exit 1
fi

###############################################################################
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - I2C Throughput Performance Test"
log "============================================================"
log "Date              : $(date)"
log "Kernel            : $(uname -r)"
log "I2C Bus           : ${I2C_BUS}"
log "I2C Address       : ${I2C_ADDRESS}"
log "Register          : ${I2C_REGISTER}"
log "Transactions      : ${TRANSACTION_COUNT}"
log "Bytes/Transaction : ${BYTES_PER_TRANSFER}"
log "I2C Device        : ${I2C_DEVICE}"
log "Log File          : ${LOG_FILE}"
log "============================================================"
log ""

###############################################################################
# Test 1 - I2C Subsystem
###############################################################################

log "------------------------------------------------------------"
log "TEST 1: I2C subsystem"
log "------------------------------------------------------------"

if [ -d /sys/bus/i2c ] && [ -d /sys/class/i2c-adapter ]; then

    pass "Linux I2C subsystem is available."

else

    fail "Linux I2C subsystem is not available."

    exit 1
fi

###############################################################################
# Test 2 - I2C Device
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 2: I2C device verification"
log "------------------------------------------------------------"

if [ -e "${I2C_DEVICE}" ]; then

    pass "${I2C_DEVICE} exists."

else

    fail "${I2C_DEVICE} does not exist."

    log ""
    log "Available I2C devices:"

    ls -l /dev/i2c-* \
        2>/dev/null | tee -a "${LOG_FILE}" || true

    exit 1
fi

###############################################################################
# Test 3 - Required Utility
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 3: I2C utility verification"
log "------------------------------------------------------------"

if command -v i2cget >/dev/null 2>&1; then

    pass "i2cget is available."

else

    fail "i2cget is not installed."

    log ""
    log "Install the i2c-tools package before running this test."

    exit 1
fi

###############################################################################
# Test 4 - Slave Detection
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 4: I2C slave detection"
log "------------------------------------------------------------"

if command -v i2cdetect >/dev/null 2>&1; then

    log "Scanning I2C bus ${I2C_BUS}..."

    i2cdetect -y "${I2C_BUS}" \
        2>&1 | tee -a "${LOG_FILE}" || true

    ADDRESS_DEC=$((I2C_ADDRESS))

    if i2cdetect -y "${I2C_BUS}" 2>/dev/null | \
        grep -qE "(^|[[:space:]])$(printf '%02x' "${ADDRESS_DEC}")([[:space:]]|$)"; then

        pass "I2C slave ${I2C_ADDRESS} detected."

    else

        skip "I2C slave ${I2C_ADDRESS} was not detected by scan."

        log ""
        log "The throughput test may fail if the device is not connected."

    fi

else

    skip "i2cdetect is not available."

fi

###############################################################################
# Test 5 - Initial Transaction
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 5: Initial I2C transaction"
log "------------------------------------------------------------"

INITIAL_VALUE="$(
    i2cget \
        -y \
        "${I2C_BUS}" \
        "${I2C_ADDRESS}" \
        "${I2C_REGISTER}" \
        2>/dev/null || true
)"

if [[ "${INITIAL_VALUE}" =~ ^0x[0-9a-fA-F]+$ ]]; then

    log "Initial register value: ${INITIAL_VALUE}"

    pass "I2C transaction completed successfully."

else

    fail "Initial I2C transaction failed."

    log ""
    log "Check:"
    log "  - I2C bus number"
    log "  - I2C slave address"
    log "  - Register address"
    log "  - SDA/SCL wiring"
    log "  - Pull-up resistors"
    log "  - Device Tree configuration"

    exit 1
fi

###############################################################################
# Test 6 - Throughput Measurement
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 6: I2C throughput measurement"
log "------------------------------------------------------------"

SUCCESSFUL_TRANSACTIONS=0
FAILED_TRANSACTIONS=0

START_TIME="$(date +%s%N)"

for ((i=1; i<=TRANSACTION_COUNT; i++)); do

    VALUE="$(
        i2cget \
            -y \
            "${I2C_BUS}" \
            "${I2C_ADDRESS}" \
            "${I2C_REGISTER}" \
            2>/dev/null || true
    )"

    if [[ "${VALUE}" =~ ^0x[0-9a-fA-F]+$ ]]; then

        SUCCESSFUL_TRANSACTIONS=$((SUCCESSFUL_TRANSACTIONS + 1))

    else

        FAILED_TRANSACTIONS=$((FAILED_TRANSACTIONS + 1))

    fi

done

END_TIME="$(date +%s%N)"

###############################################################################
# Calculate Timing
###############################################################################

ELAPSED_NS=$((END_TIME - START_TIME))

ELAPSED_US=$((ELAPSED_NS / 1000))

ELAPSED_MS=$((ELAPSED_NS / 1000000))

if [ "${ELAPSED_US}" -gt 0 ]; then

    TRANSACTIONS_PER_SEC=$(
        awk "BEGIN {
            printf \"%.2f\", ${SUCCESSFUL_TRANSACTIONS} * 1000000 / ${ELAPSED_US}
        }"
    )

else

    TRANSACTIONS_PER_SEC="0.00"

fi

###############################################################################
# Calculate Data Throughput
###############################################################################

TOTAL_BYTES=$(
    awk "BEGIN {
        printf \"%.0f\", ${SUCCESSFUL_TRANSACTIONS} * ${BYTES_PER_TRANSFER}
    }"
)

if [ "${ELAPSED_US}" -gt 0 ]; then

    BYTES_PER_SEC=$(
        awk "BEGIN {
            printf \"%.2f\", ${TOTAL_BYTES} * 1000000 / ${ELAPSED_US}
        }"
    )

    KB_PER_SEC=$(
        awk "BEGIN {
            printf \"%.2f\", ${TOTAL_BYTES} * 1000000 / ${ELAPSED_US} / 1024
        }"
    )

else

    BYTES_PER_SEC="0.00"
    KB_PER_SEC="0.00"

fi

###############################################################################
# Results
###############################################################################

log ""
log "I2C Throughput Results"
log "----------------------"
log "Requested transactions : ${TRANSACTION_COUNT}"
log "Successful transactions: ${SUCCESSFUL_TRANSACTIONS}"
log "Failed transactions    : ${FAILED_TRANSACTIONS}"
log "Bytes per transaction  : ${BYTES_PER_TRANSFER}"
log "Total bytes            : ${TOTAL_BYTES}"
log "Elapsed time           : ${ELAPSED_MS} ms"
log "Transaction rate       : ${TRANSACTIONS_PER_SEC} transactions/sec"
log "Effective throughput   : ${BYTES_PER_SEC} bytes/sec"
log "Effective throughput   : ${KB_PER_SEC} KB/sec"

if [ "${SUCCESSFUL_TRANSACTIONS}" -eq "${TRANSACTION_COUNT}" ]; then

    pass "All I2C transactions completed successfully."

else

    fail "One or more I2C transactions failed."

fi

###############################################################################
# Test 7 - Transaction Latency
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 7: I2C transaction latency"
log "------------------------------------------------------------"

if [ "${SUCCESSFUL_TRANSACTIONS}" -gt 0 ]; then

    AVG_LATENCY_US=$(
        awk "BEGIN {
            printf \"%.2f\", ${ELAPSED_US} / ${SUCCESSFUL_TRANSACTIONS}
        }"
    )

    log "Average transaction latency: ${AVG_LATENCY_US} us"

    pass "Average I2C transaction latency calculated."

else

    skip "Unable to calculate transaction latency."

fi

###############################################################################
# Test 8 - I2C Bus Information
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 8: I2C bus information"
log "------------------------------------------------------------"

I2C_SYSFS="/sys/class/i2c-adapter/i2c-${I2C_BUS}"

if [ -d "${I2C_SYSFS}" ]; then

    log "I2C sysfs path: ${I2C_SYSFS}"

    if [ -f "${I2C_SYSFS}/name" ]; then

        log "Adapter name: $(cat "${I2C_SYSFS}/name")"

    fi

    pass "I2C bus information inspected."

else

    skip "I2C sysfs information not available."

fi

###############################################################################
# Test 9 - Kernel Messages
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 9: I2C kernel messages"
log "------------------------------------------------------------"

dmesg 2>/dev/null | \
    grep -iE "i2c|omap.*i2c|twsi" | \
    tail -30 | \
    tee -a "${LOG_FILE}" || true

pass "I2C kernel log inspection completed."

###############################################################################
# Summary
###############################################################################

log ""
log "============================================================"
log " I2C THROUGHPUT PERFORMANCE SUMMARY"
log "============================================================"

echo "PASS : ${PASS}" | tee -a "${LOG_FILE}"
echo "FAIL : ${FAIL}" | tee -a "${LOG_FILE}"
echo "SKIP : ${SKIP}" | tee -a "${LOG_FILE}"

log ""
log "I2C Bus              : ${I2C_BUS}"
log "I2C Address          : ${I2C_ADDRESS}"
log "Register             : ${I2C_REGISTER}"
log "Requested Transactions: ${TRANSACTION_COUNT}"
log "Successful Transactions: ${SUCCESSFUL_TRANSACTIONS}"
log "Failed Transactions    : ${FAILED_TRANSACTIONS}"
log "Total Bytes            : ${TOTAL_BYTES}"
log "Elapsed Time           : ${ELAPSED_MS} ms"
log "Transaction Rate       : ${TRANSACTIONS_PER_SEC} transactions/sec"
log "Effective Throughput   : ${BYTES_PER_SEC} bytes/sec"
log "Effective Throughput   : ${KB_PER_SEC} KB/sec"
log "Average Latency        : ${AVG_LATENCY_US:-N/A} us"

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
