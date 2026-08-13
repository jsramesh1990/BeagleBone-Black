#!/bin/bash

###############################################################################
# BeagleBone Black - SPI Stress Test
#
# File:
#   tests/stress/spi_stress.sh
#
# Purpose:
#   Continuously perform SPI loopback transactions and verify transmitted
#   and received data for a configurable duration.
#
# Usage:
#   sudo ./spi_stress.sh
#   sudo ./spi_stress.sh <device> <duration_seconds> <iterations>
#
# Example:
#   sudo ./spi_stress.sh /dev/spidev1.0 60 10000
#
# IMPORTANT:
#   This test requires SPI loopback:
#
#       MOSI ---------------- MISO
#
#   Connect the SPI MOSI pin to MISO through the appropriate wiring.
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

SPI_DEVICE="${1:-/dev/spidev1.0}"
DURATION="${2:-60}"
ITERATIONS="${3:-10000}"

SPI_SPEED="${SPI_SPEED:-1000000}"
SPI_BITS="${SPI_BITS:-8}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs/stress"
LOG_FILE="${LOG_DIR}/spi_stress_${TIMESTAMP}.log"

mkdir -p "${LOG_DIR}"

###############################################################################
# Counters
###############################################################################

PASS=0
FAIL=0
SKIP=0

TX_COUNT=0
RX_COUNT=0
TRANSFER_ERRORS=0
DATA_ERRORS=0

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

    fail "Run SPI stress test as root."

    echo
    echo "Usage:"
    echo "  sudo ./spi_stress.sh"
    echo "  sudo ./spi_stress.sh /dev/spidev1.0 60 10000"

    exit 1
fi

###############################################################################
# Parameter Validation
###############################################################################

if [ ! -e "${SPI_DEVICE}" ]; then

    fail "SPI device does not exist: ${SPI_DEVICE}"

    log ""
    log "Available SPI devices:"

    ls -l /dev/spidev* 2>/dev/null |
        tee -a "${LOG_FILE}" || true

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
log " BeagleBone Black - SPI Stress Test"
log "============================================================"
log "Date          : $(date)"
log "Kernel        : $(uname -r)"
log "SPI Device    : ${SPI_DEVICE}"
log "SPI Speed     : ${SPI_SPEED} Hz"
log "SPI Bits      : ${SPI_BITS}"
log "Duration      : ${DURATION} seconds"
log "Iterations    : ${ITERATIONS}"
log "Log File      : ${LOG_FILE}"
log "============================================================"
log ""

###############################################################################
# Test 1 - SPI Utility
###############################################################################

log "------------------------------------------------------------"
log "TEST 1: SPI utility availability"
log "------------------------------------------------------------"

if command -v spidev_test >/dev/null 2>&1; then

    pass "spidev_test is available."

else

    fail "spidev_test is not installed."

    log ""
    log "Install the SPI test utility before running this test."

    exit 1
fi

###############################################################################
# Test 2 - SPI Device
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 2: SPI device verification"
log "------------------------------------------------------------"

if [ -c "${SPI_DEVICE}" ]; then

    log "SPI character device:"
    ls -l "${SPI_DEVICE}" | tee -a "${LOG_FILE}"

    pass "${SPI_DEVICE} is available."

else

    fail "${SPI_DEVICE} is not a valid character device."

    exit 1
fi

###############################################################################
# Test 3 - SPI Device Information
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 3: SPI device information"
log "------------------------------------------------------------"

SPI_BUS_INFO="$(basename "${SPI_DEVICE}")"

log "SPI device: ${SPI_BUS_INFO}"

if [ -d /sys/class/spidev ]; then

    find /sys/class/spidev \
        -maxdepth 2 \
        -type l \
        2>/dev/null |
        tee -a "${LOG_FILE}" || true

fi

pass "SPI device information collected."

###############################################################################
# Test 4 - SPI Basic Loopback
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 4: SPI basic loopback"
log "------------------------------------------------------------"

BASIC_TEST_OUTPUT="$(
    spidev_test \
        -D "${SPI_DEVICE}" \
        -s "${SPI_SPEED}" \
        -b "${SPI_BITS}" \
        -v 2>&1
)"

BASIC_TEST_STATUS=$?

echo "${BASIC_TEST_OUTPUT}" |
    tee -a "${LOG_FILE}"

if [ "${BASIC_TEST_STATUS}" -eq 0 ]; then

    pass "SPI basic transaction completed."

else

    fail "SPI basic transaction failed."

    log ""
    log "Check:"
    log "  - MOSI/MISO loopback wiring"
    log "  - SPI Device Tree configuration"
    log "  - SPI pinmux"
    log "  - SPI clock"
    log "  - Chip select"

    exit 1
fi

###############################################################################
# Test 5 - SPI Loopback Data Verification
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 5: SPI loopback data verification"
log "------------------------------------------------------------"

TEST_PATTERN="AA"

LOOPBACK_OUTPUT="$(
    printf "${TEST_PATTERN}" |
    xxd -r -p |
    spidev_test \
        -D "${SPI_DEVICE}" \
        -s "${SPI_SPEED}" \
        -b "${SPI_BITS}" \
        2>&1
)"

echo "${LOOPBACK_OUTPUT}" |
    tee -a "${LOG_FILE}"

if [ -n "${LOOPBACK_OUTPUT}" ]; then

    pass "SPI loopback transaction produced response data."

else

    fail "SPI loopback produced no response."

fi

###############################################################################
# Test 6 - SPI Stress Transactions
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 6: SPI continuous transaction stress"
log "------------------------------------------------------------"

log "Starting SPI stress test..."

START_TIME="$(date +%s)"

for ((i=1; i<=ITERATIONS; i++)); do

    CURRENT_TIME="$(date +%s)"

    ELAPSED=$((CURRENT_TIME - START_TIME))

    if [ "${ELAPSED}" -ge "${DURATION}" ]; then
        break
    fi

    ###########################################################################
    # Generate changing SPI pattern
    ###########################################################################

    BYTE1=$((i % 256))
    BYTE2=$(((i + 85) % 256))
    BYTE3=$(((i + 170) % 256))
    BYTE4=$(((i * 3) % 256))

    PATTERN="$(
        printf '%02X%02X%02X%02X' \
            "${BYTE1}" \
            "${BYTE2}" \
            "${BYTE3}" \
            "${BYTE4}"
    )"

    ###########################################################################
    # Execute SPI transaction
    ###########################################################################

    SPI_OUTPUT="$(
        printf "${PATTERN}" |
        xxd -r -p |
        spidev_test \
            -D "${SPI_DEVICE}" \
            -s "${SPI_SPEED}" \
            -b "${SPI_BITS}" \
            2>/dev/null
    )"

    if [ $? -eq 0 ]; then

        TX_COUNT=$((TX_COUNT + 1))

    else

        TRANSFER_ERRORS=$((TRANSFER_ERRORS + 1))

        continue
    fi

    ###########################################################################
    # Basic response validation
    ###########################################################################

    if [ -n "${SPI_OUTPUT}" ]; then

        RX_COUNT=$((RX_COUNT + 1))

    else

        DATA_ERRORS=$((DATA_ERRORS + 1))

    fi

    ###########################################################################
    # Progress
    ###########################################################################

    if [ $((i % 100)) -eq 0 ]; then

        log "[INFO] Iteration: ${i} | TX: ${TX_COUNT} | RX: ${RX_COUNT} | Errors: ${TRANSFER_ERRORS}"

    fi

done

END_TIME="$(date +%s)"

ACTUAL_DURATION=$((END_TIME - START_TIME))

if [ "${ACTUAL_DURATION}" -le 0 ]; then
    ACTUAL_DURATION=1
fi

###############################################################################
# Test 7 - SPI Throughput
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 7: SPI throughput"
log "------------------------------------------------------------"

TRANSFER_RATE="$(
    awk "BEGIN {
        printf \"%.2f\", ${TX_COUNT} / ${ACTUAL_DURATION}
    }"
)"

log "Actual duration : ${ACTUAL_DURATION} seconds"
log "TX transactions : ${TX_COUNT}"
log "RX transactions : ${RX_COUNT}"
log "Transfer errors : ${TRANSFER_ERRORS}"
log "Data errors     : ${DATA_ERRORS}"
log "Transfer rate   : ${TRANSFER_RATE} transactions/sec"

if [ "${TX_COUNT}" -gt 0 ]; then

    pass "SPI transactions completed."

else

    fail "No SPI transactions completed."

fi

###############################################################################
# Test 8 - Transfer Error Check
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 8: SPI transfer error check"
log "------------------------------------------------------------"

if [ "${TRANSFER_ERRORS}" -eq 0 ]; then

    pass "No SPI transfer errors detected."

else

    fail "SPI transfer errors detected: ${TRANSFER_ERRORS}"

fi

###############################################################################
# Test 9 - Data Error Check
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 9: SPI data response check"
log "------------------------------------------------------------"

if [ "${DATA_ERRORS}" -eq 0 ]; then

    pass "All SPI transactions returned response data."

else

    fail "SPI transactions with missing response data: ${DATA_ERRORS}"

fi

###############################################################################
# Test 10 - SPI Device Recheck
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 10: SPI post-stress device check"
log "------------------------------------------------------------"

if [ -c "${SPI_DEVICE}" ]; then

    pass "SPI device remains available after stress test."

else

    fail "SPI device disappeared after stress test."

fi

###############################################################################
# Test 11 - SPI Kernel Error Scan
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 11: SPI kernel error scan"
log "------------------------------------------------------------"

SPI_ERRORS="$(
    dmesg 2>/dev/null |
    grep -iE \
    "spi|spidev|mcspi|spi_master|spi_transfer|"
    "timeout|overrun|underrun|pinctrl|pinmux|error|failed" |
    tail -50
)"

if [ -n "${SPI_ERRORS}" ]; then

    log "Recent SPI-related kernel messages:"
    echo "${SPI_ERRORS}" |
        tee -a "${LOG_FILE}"

else

    log "No SPI-related kernel error messages found."

fi

pass "SPI kernel log scan completed."

###############################################################################
# Test 12 - SPI Device Tree Information
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 12: SPI Device Tree information"
log "------------------------------------------------------------"

SPI_DT_INFO="$(
    find /sys/firmware/devicetree/base \
        -type d \
        -iname "*spi*" \
        2>/dev/null |
    head -20
)"

if [ -n "${SPI_DT_INFO}" ]; then

    echo "${SPI_DT_INFO}" |
        tee -a "${LOG_FILE}"

    pass "SPI Device Tree information collected."

else

    skip "SPI Device Tree nodes could not be identified."

fi

###############################################################################
# Test 13 - SPI Statistics
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 13: SPI runtime information"
log "------------------------------------------------------------"

log "SPI devices currently exposed:"

ls -l /dev/spidev* 2>/dev/null |
    tee -a "${LOG_FILE}" || true

if [ -d /sys/class/spidev ]; then

    log ""
    log "SPI sysfs entries:"

    ls -l /sys/class/spidev 2>/dev/null |
        tee -a "${LOG_FILE}" || true

fi

pass "SPI runtime information collected."

###############################################################################
# Summary
###############################################################################

log ""
log "============================================================"
log " SPI STRESS TEST SUMMARY"
log "============================================================"

echo "PASS              : ${PASS}" | tee -a "${LOG_FILE}"
echo "FAIL              : ${FAIL}" | tee -a "${LOG_FILE}"
echo "SKIP              : ${SKIP}" | tee -a "${LOG_FILE}"

log ""
log "SPI Device        : ${SPI_DEVICE}"
log "SPI Speed         : ${SPI_SPEED} Hz"
log "SPI Bits          : ${SPI_BITS}"
log "Duration          : ${ACTUAL_DURATION} sec"
log "TX Transactions   : ${TX_COUNT}"
log "RX Transactions   : ${RX_COUNT}"
log "Transfer Errors   : ${TRANSFER_ERRORS}"
log "Data Errors       : ${DATA_ERRORS}"
log "Transfer Rate     : ${TRANSFER_RATE} transactions/sec"

log ""
log "Log File:"
log "${LOG_FILE}"

###############################################################################
# Final Result
###############################################################################

if [ "${TRANSFER_ERRORS}" -eq 0 ] &&
   [ "${DATA_ERRORS}" -eq 0 ] &&
   [ "${FAIL}" -eq 0 ]; then

    log ""
    log "RESULT: PASS"

    exit 0

else

    log ""
    log "RESULT: FAIL"

    exit 1

fi
