#!/bin/bash

###############################################################################
# BeagleBone Black - SPI Throughput Performance Test
#
# File:
#   tests/performance/spi_throughput.sh
#
# Purpose:
#   Measure SPI transfer throughput, transaction latency, and transfer
#   success rate using the Linux spidev interface.
#
# Usage:
#   sudo ./spi_throughput.sh
#   sudo ./spi_throughput.sh <device> <speed_hz> <bytes> <count>
#
# Example:
#   sudo ./spi_throughput.sh /dev/spidev1.0 1000000 256 100
#
# Parameters:
#   device     - SPI device node
#   speed_hz   - SPI clock frequency
#   bytes      - Bytes transferred per transaction
#   count      - Number of transactions
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

SPI_DEVICE="${1:-/dev/spidev1.0}"
SPI_SPEED="${2:-1000000}"
BYTES_PER_TRANSFER="${3:-256}"
TRANSFER_COUNT="${4:-100}"

SPI_BITS="${SPI_BITS:-8}"
SPI_MODE="${SPI_MODE:-0}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs/performance"
LOG_FILE="${LOG_DIR}/spi_throughput_${TIMESTAMP}.log"

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
    echo "  sudo ./spi_throughput.sh"

    exit 1
fi

###############################################################################
# Parameter Validation
###############################################################################

if [ "${SPI_SPEED}" -le 0 ]; then
    fail "SPI speed must be greater than zero."
    exit 1
fi

if [ "${BYTES_PER_TRANSFER}" -le 0 ]; then
    fail "Transfer size must be greater than zero."
    exit 1
fi

if [ "${TRANSFER_COUNT}" -le 0 ]; then
    fail "Transfer count must be greater than zero."
    exit 1
fi

###############################################################################
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - SPI Throughput Performance Test"
log "============================================================"
log "Date              : $(date)"
log "Kernel            : $(uname -r)"
log "SPI Device        : ${SPI_DEVICE}"
log "SPI Mode          : ${SPI_MODE}"
log "SPI Speed         : ${SPI_SPEED} Hz"
log "Bits per Word     : ${SPI_BITS}"
log "Bytes/Transfer    : ${BYTES_PER_TRANSFER}"
log "Transfer Count    : ${TRANSFER_COUNT}"
log "Log File          : ${LOG_FILE}"
log "============================================================"
log ""

###############################################################################
# Test 1 - SPI Subsystem
###############################################################################

log "------------------------------------------------------------"
log "TEST 1: SPI subsystem"
log "------------------------------------------------------------"

if [ -d /sys/bus/spi ] && [ -d /sys/class/spi_master ]; then

    pass "Linux SPI subsystem is available."

else

    fail "Linux SPI subsystem is not available."

    exit 1
fi

###############################################################################
# Test 2 - SPI Device
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 2: SPI device verification"
log "------------------------------------------------------------"

if [ -e "${SPI_DEVICE}" ]; then

    pass "${SPI_DEVICE} exists."

else

    fail "${SPI_DEVICE} does not exist."

    log ""
    log "Available SPI devices:"

    ls -l /dev/spidev* \
        2>/dev/null | tee -a "${LOG_FILE}" || true

    exit 1
fi

if [ ! -c "${SPI_DEVICE}" ]; then

    fail "${SPI_DEVICE} is not a character device."

    exit 1
fi

###############################################################################
# Test 3 - spidev_test Verification
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 3: SPI test utility"
log "------------------------------------------------------------"

if command -v spidev_test >/dev/null 2>&1; then

    SPIDEV_TEST="$(command -v spidev_test)"

    log "spidev_test: ${SPIDEV_TEST}"

    pass "spidev_test is available."

else

    fail "spidev_test is not installed."

    log ""
    log "Install/build the Linux spidev_test utility."

    exit 1
fi

###############################################################################
# Test 4 - SPI Controller Information
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 4: SPI controller information"
log "------------------------------------------------------------"

SPI_DEV_NAME="$(basename "${SPI_DEVICE}")"

log "SPI device: ${SPI_DEV_NAME}"

if [ -d "/sys/class/spidev/${SPI_DEV_NAME}" ]; then

    log "SPI sysfs entry found."

    pass "SPI controller/device information available."

else

    skip "SPI spidev sysfs entry not found."

fi

###############################################################################
# Test 5 - Initial SPI Transaction
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 5: Initial SPI transaction"
log "------------------------------------------------------------"

log "Performing initial SPI transfer..."

if spidev_test \
    -D "${SPI_DEVICE}" \
    -s "${SPI_SPEED}" \
    -b "${SPI_BITS}" \
    -m "${SPI_MODE}" \
    -p "BBB_SPI_TEST" \
    2>&1 | tee -a "${LOG_FILE}"; then

    pass "Initial SPI transaction completed."

else

    fail "Initial SPI transaction failed."

    log ""
    log "Check:"
    log "  - SPI device connection"
    log "  - SPI chip-select"
    log "  - SPI pinmux"
    log "  - SPI Device Tree configuration"
    log "  - SPI clock configuration"

    exit 1
fi

###############################################################################
# Test 6 - SPI Throughput Measurement
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 6: SPI throughput measurement"
log "------------------------------------------------------------"

SUCCESSFUL_TRANSFERS=0
FAILED_TRANSFERS=0

PAYLOAD="$(printf 'A%.0s' $(seq 1 "${BYTES_PER_TRANSFER}"))"

START_TIME="$(date +%s%N)"

for ((i=1; i<=TRANSFER_COUNT; i++)); do

    if spidev_test \
        -D "${SPI_DEVICE}" \
        -s "${SPI_SPEED}" \
        -b "${SPI_BITS}" \
        -m "${SPI_MODE}" \
        -p "${PAYLOAD}" \
        >/dev/null 2>&1; then

        SUCCESSFUL_TRANSFERS=$((SUCCESSFUL_TRANSFERS + 1))

    else

        FAILED_TRANSFERS=$((FAILED_TRANSFERS + 1))

    fi

done

END_TIME="$(date +%s%N)"

###############################################################################
# Timing Calculations
###############################################################################

ELAPSED_NS=$((END_TIME - START_TIME))

ELAPSED_US=$((ELAPSED_NS / 1000))

ELAPSED_MS=$((ELAPSED_NS / 1000000))

TOTAL_BYTES=$(
    awk "BEGIN {
        printf \"%.0f\", ${SUCCESSFUL_TRANSFERS} * ${BYTES_PER_TRANSFER}
    }"
)

if [ "${ELAPSED_US}" -gt 0 ]; then

    TRANSFERS_PER_SEC=$(
        awk "BEGIN {
            printf \"%.2f\", ${SUCCESSFUL_TRANSFERS} * 1000000 / ${ELAPSED_US}
        }"
    )

    BYTES_PER_SEC=$(
        awk "BEGIN {
            printf \"%.2f\", ${TOTAL_BYTES} * 1000000 / ${ELAPSED_US}
        }"
    )

    MB_PER_SEC=$(
        awk "BEGIN {
            printf \"%.3f\", ${TOTAL_BYTES} * 1000000 / ${ELAPSED_US} / 1000000
        }"
    )

    MBPS=$(
        awk "BEGIN {
            printf \"%.3f\", ${TOTAL_BYTES} * 8 / ${ELAPSED_US} / 1000
        }"
    )

else

    TRANSFERS_PER_SEC="0.00"
    BYTES_PER_SEC="0.00"
    MB_PER_SEC="0.000"
    MBPS="0.000"

fi

###############################################################################
# Results
###############################################################################

log ""
log "SPI Throughput Results"
log "----------------------"
log "Requested transfers : ${TRANSFER_COUNT}"
log "Successful transfers: ${SUCCESSFUL_TRANSFERS}"
log "Failed transfers    : ${FAILED_TRANSFERS}"
log "Bytes/transfer      : ${BYTES_PER_TRANSFER}"
log "Total bytes         : ${TOTAL_BYTES}"
log "Elapsed time        : ${ELAPSED_MS} ms"
log "Transfer rate       : ${TRANSFERS_PER_SEC} transfers/sec"
log "Throughput          : ${BYTES_PER_SEC} bytes/sec"
log "Throughput          : ${MB_PER_SEC} MB/sec"
log "Data rate           : ${MBPS} Mbit/sec"

if [ "${SUCCESSFUL_TRANSFERS}" -eq "${TRANSFER_COUNT}" ]; then

    pass "All SPI transfers completed successfully."

else

    fail "One or more SPI transfers failed."

fi

###############################################################################
# Test 7 - Average Transaction Latency
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 7: SPI transaction latency"
log "------------------------------------------------------------"

if [ "${SUCCESSFUL_TRANSFERS}" -gt 0 ]; then

    AVG_LATENCY_US=$(
        awk "BEGIN {
            printf \"%.2f\", ${ELAPSED_US} / ${SUCCESSFUL_TRANSFERS}
        }"
    )

    log "Average transaction latency: ${AVG_LATENCY_US} us"

    pass "SPI transaction latency calculated."

else

    skip "Unable to calculate SPI transaction latency."

fi

###############################################################################
# Test 8 - Theoretical Bus Rate
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 8: SPI theoretical data rate"
log "------------------------------------------------------------"

THEORETICAL_MBPS=$(
    awk "BEGIN {
        printf \"%.3f\", ${SPI_SPEED} * ${SPI_BITS} / 1000000
    }"
)

log "Configured SPI clock       : ${SPI_SPEED} Hz"
log "Bits per word              : ${SPI_BITS}"
log "Theoretical data rate      : ${THEORETICAL_MBPS} Mbit/sec"

pass "Theoretical SPI data rate calculated."

###############################################################################
# Test 9 - Kernel SPI Messages
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 9: SPI kernel messages"
log "------------------------------------------------------------"

dmesg 2>/dev/null | \
    grep -iE "spi|spidev|mcspi|omap.*spi" | \
    tail -30 | \
    tee -a "${LOG_FILE}" || true

pass "SPI kernel log inspection completed."

###############################################################################
# Summary
###############################################################################

log ""
log "============================================================"
log " SPI THROUGHPUT PERFORMANCE SUMMARY"
log "============================================================"

echo "PASS : ${PASS}" | tee -a "${LOG_FILE}"
echo "FAIL : ${FAIL}" | tee -a "${LOG_FILE}"
echo "SKIP : ${SKIP}" | tee -a "${LOG_FILE}"

log ""
log "SPI Device           : ${SPI_DEVICE}"
log "SPI Mode             : ${SPI_MODE}"
log "SPI Clock            : ${SPI_SPEED} Hz"
log "Bits per Word        : ${SPI_BITS}"
log "Bytes/Transfer       : ${BYTES_PER_TRANSFER}"
log "Transfer Count       : ${TRANSFER_COUNT}"
log "Successful Transfers : ${SUCCESSFUL_TRANSFERS}"
log "Failed Transfers     : ${FAILED_TRANSFERS}"
log "Total Bytes          : ${TOTAL_BYTES}"
log "Elapsed Time         : ${ELAPSED_MS} ms"
log "Transfer Rate        : ${TRANSFERS_PER_SEC} transfers/sec"
log "Throughput           : ${MB_PER_SEC} MB/sec"
log "Data Rate            : ${MBPS} Mbit/sec"
log "Avg Transaction Time : ${AVG_LATENCY_US:-N/A} us"
log "Theoretical Rate     : ${THEORETICAL_MBPS} Mbit/sec"

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
