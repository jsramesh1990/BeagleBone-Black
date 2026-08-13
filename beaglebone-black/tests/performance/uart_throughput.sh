#!/bin/bash

###############################################################################
# BeagleBone Black - UART Throughput Performance Test
#
# File:
#   tests/performance/uart_throughput.sh
#
# Purpose:
#   Measure UART transmit throughput, transfer latency, and data integrity
#   using a UART loopback connection.
#
# IMPORTANT:
#   Connect TX and RX of the selected UART together for loopback testing.
#
# Usage:
#   sudo ./uart_throughput.sh
#   sudo ./uart_throughput.sh <device> <baudrate> <bytes> <count>
#
# Example:
#   sudo ./uart_throughput.sh /dev/ttyS1 115200 256 100
#
# Parameters:
#   device    - UART device node
#   baudrate  - UART baud rate
#   bytes     - Bytes per transfer
#   count     - Number of transfers
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

UART_DEVICE="${1:-/dev/ttyS1}"
BAUDRATE="${2:-115200}"
BYTES_PER_TRANSFER="${3:-256}"
TRANSFER_COUNT="${4:-100}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs/performance"
LOG_FILE="${LOG_DIR}/uart_throughput_${TIMESTAMP}.log"

TMP_FILE="/tmp/uart_loopback_${$}.bin"
RX_FILE="/tmp/uart_rx_${$}.bin"

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
# Cleanup
###############################################################################

cleanup()
{
    rm -f "${TMP_FILE}"
    rm -f "${RX_FILE}"

    stty -F "${UART_DEVICE}" sane 2>/dev/null || true
}

trap cleanup EXIT

###############################################################################
# Root Check
###############################################################################

if [ "${EUID}" -ne 0 ]; then

    fail "Run this test as root."

    echo
    echo "Usage:"
    echo "  sudo ./uart_throughput.sh"

    exit 1
fi

###############################################################################
# Parameter Validation
###############################################################################

if [ "${BAUDRATE}" -le 0 ]; then

    fail "Baud rate must be greater than zero."

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
log " BeagleBone Black - UART Throughput Performance Test"
log "============================================================"
log "Date              : $(date)"
log "Kernel            : $(uname -r)"
log "UART Device       : ${UART_DEVICE}"
log "Baud Rate         : ${BAUDRATE}"
log "Bytes/Transfer    : ${BYTES_PER_TRANSFER}"
log "Transfer Count    : ${TRANSFER_COUNT}"
log "Log File          : ${LOG_FILE}"
log "============================================================"
log ""

###############################################################################
# Test 1 - UART Device
###############################################################################

log "------------------------------------------------------------"
log "TEST 1: UART device verification"
log "------------------------------------------------------------"

if [ -e "${UART_DEVICE}" ]; then

    pass "${UART_DEVICE} exists."

else

    fail "${UART_DEVICE} does not exist."

    log ""
    log "Available UART devices:"

    ls -l /dev/ttyS* /dev/ttyAMA* /dev/ttyUSB* /dev/ttyACM* \
        2>/dev/null | tee -a "${LOG_FILE}" || true

    exit 1
fi

if [ ! -c "${UART_DEVICE}" ]; then

    fail "${UART_DEVICE} is not a character device."

    exit 1
fi

###############################################################################
# Test 2 - UART Configuration
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 2: UART configuration"
log "------------------------------------------------------------"

if stty -F "${UART_DEVICE}" \
    "${BAUDRATE}" \
    cs8 \
    -cstopb \
    -parenb \
    -ixon \
    -ixoff \
    -crtscts \
    -echo \
    -icanon \
    min 0 \
    time 1 \
    2>/dev/null; then

    pass "UART configured successfully."

else

    fail "Unable to configure UART."

    exit 1
fi

###############################################################################
# Display Configuration
###############################################################################

log ""
log "UART configuration:"
stty -F "${UART_DEVICE}" 2>/dev/null | tee -a "${LOG_FILE}" || true

###############################################################################
# Test 3 - Loopback Preparation
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 3: UART loopback preparation"
log "------------------------------------------------------------"

log "IMPORTANT:"
log "Connect UART TX to UART RX for loopback testing."
log ""
log "Expected:"
log "  TX -------------- RX"
log ""
log "Waiting 2 seconds before starting..."

sleep 2

pass "UART loopback test preparation completed."

###############################################################################
# Test 4 - Basic Loopback
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 4: Basic UART loopback"
log "------------------------------------------------------------"

TEST_STRING="BBB_UART_LOOPBACK_TEST"

printf "%s" "${TEST_STRING}" > "${TMP_FILE}"

rm -f "${RX_FILE}"

timeout 5 cat "${UART_DEVICE}" > "${RX_FILE}" &
RX_PID=$!

sleep 0.2

cat "${TMP_FILE}" > "${UART_DEVICE}"

sleep 1

kill "${RX_PID}" 2>/dev/null || true
wait "${RX_PID}" 2>/dev/null || true

if [ -f "${RX_FILE}" ] && \
   grep -q "${TEST_STRING}" "${RX_FILE}" 2>/dev/null; then

    pass "Basic UART loopback successful."

else

    fail "Basic UART loopback failed."

    log ""
    log "Check TX/RX wiring and UART configuration."

    exit 1
fi

###############################################################################
# Test 5 - Throughput Measurement
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 5: UART throughput measurement"
log "------------------------------------------------------------"

SUCCESSFUL_TRANSFERS=0
FAILED_TRANSFERS=0
TOTAL_BYTES=0

PAYLOAD_FILE="/tmp/uart_payload_${$}.bin"

# Generate deterministic test data.
if command -v python3 >/dev/null 2>&1; then

    python3 - "${BYTES_PER_TRANSFER}" "${PAYLOAD_FILE}" <<'PY'
import sys

size = int(sys.argv[1])
path = sys.argv[2]

data = bytes((i % 256 for i in range(size)))

with open(path, "wb") as f:
    f.write(data)
PY

else

    dd if=/dev/zero \
       of="${PAYLOAD_FILE}" \
       bs=1 \
       count="${BYTES_PER_TRANSFER}" \
       2>/dev/null

fi

START_TIME="$(date +%s%N)"

for ((i=1; i<=TRANSFER_COUNT; i++)); do

    rm -f "${RX_FILE}"

    timeout 10 cat "${UART_DEVICE}" > "${RX_FILE}" &
    RX_PID=$!

    sleep 0.05

    cat "${PAYLOAD_FILE}" > "${UART_DEVICE}"

    # Allow the final bytes to arrive.
    sleep 0.05

    kill "${RX_PID}" 2>/dev/null || true
    wait "${RX_PID}" 2>/dev/null || true

    if [ -f "${RX_FILE}" ]; then

        RECEIVED_BYTES="$(wc -c < "${RX_FILE}")"

    else

        RECEIVED_BYTES=0

    fi

    if [ "${RECEIVED_BYTES}" -eq "${BYTES_PER_TRANSFER}" ] && \
       cmp -s "${PAYLOAD_FILE}" "${RX_FILE}"; then

        SUCCESSFUL_TRANSFERS=$((SUCCESSFUL_TRANSFERS + 1))

        TOTAL_BYTES=$((TOTAL_BYTES + BYTES_PER_TRANSFER))

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

    KB_PER_SEC=$(
        awk "BEGIN {
            printf \"%.2f\", ${TOTAL_BYTES} * 1000000 / ${ELAPSED_US} / 1024
        }"
    )

else

    TRANSFERS_PER_SEC="0.00"
    BYTES_PER_SEC="0.00"
    KB_PER_SEC="0.00"

fi

###############################################################################
# Results
###############################################################################

log ""
log "UART Throughput Results"
log "-----------------------"
log "Requested transfers : ${TRANSFER_COUNT}"
log "Successful transfers: ${SUCCESSFUL_TRANSFERS}"
log "Failed transfers    : ${FAILED_TRANSFERS}"
log "Bytes/transfer      : ${BYTES_PER_TRANSFER}"
log "Total bytes         : ${TOTAL_BYTES}"
log "Elapsed time        : ${ELAPSED_MS} ms"
log "Transfer rate       : ${TRANSFERS_PER_SEC} transfers/sec"
log "Throughput          : ${BYTES_PER_SEC} bytes/sec"
log "Throughput          : ${KB_PER_SEC} KB/sec"

if [ "${SUCCESSFUL_TRANSFERS}" -eq "${TRANSFER_COUNT}" ]; then

    pass "All UART transfers completed successfully."

else

    fail "One or more UART transfers failed."

fi

###############################################################################
# Test 6 - Data Integrity
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 6: UART data integrity"
log "------------------------------------------------------------"

if [ "${SUCCESSFUL_TRANSFERS}" -gt 0 ]; then

    pass "UART loopback data integrity verified."

else

    fail "UART data integrity verification failed."

fi

###############################################################################
# Test 7 - Average Transfer Latency
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 7: UART transfer latency"
log "------------------------------------------------------------"

if [ "${SUCCESSFUL_TRANSFERS}" -gt 0 ]; then

    AVG_LATENCY_US=$(
        awk "BEGIN {
            printf \"%.2f\", ${ELAPSED_US} / ${SUCCESSFUL_TRANSFERS}
        }"
    )

    log "Average transfer latency: ${AVG_LATENCY_US} us"

    pass "Average UART transfer latency calculated."

else

    skip "Unable to calculate transfer latency."

fi

###############################################################################
# Test 8 - Theoretical UART Data Rate
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 8: UART theoretical data rate"
log "------------------------------------------------------------"

# 8N1 = 1 start + 8 data + 1 stop = 10 bits/byte.

THEORETICAL_BYTES_SEC=$(
    awk "BEGIN {
        printf \"%.2f\", ${BAUDRATE} / 10
    }"
)

THEORETICAL_KB_SEC=$(
    awk "BEGIN {
        printf \"%.2f\", ${BAUDRATE} / 10 / 1024
    }"
)

log "Baud rate               : ${BAUDRATE} baud"
log "UART format             : 8N1"
log "Approx. bits/byte       : 10"
log "Theoretical throughput  : ${THEORETICAL_BYTES_SEC} bytes/sec"
log "Theoretical throughput  : ${THEORETICAL_KB_SEC} KB/sec"

pass "Theoretical UART data rate calculated."

###############################################################################
# Test 9 - UART Kernel Messages
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 9: UART kernel messages"
log "------------------------------------------------------------"

dmesg 2>/dev/null | \
    grep -iE "tty|uart|serial|omap.*serial|8250" | \
    tail -30 | \
    tee -a "${LOG_FILE}" || true

pass "UART kernel log inspection completed."

###############################################################################
# Cleanup Payload
###############################################################################

rm -f "${PAYLOAD_FILE}"

###############################################################################
# Summary
###############################################################################

log ""
log "============================================================"
log " UART THROUGHPUT PERFORMANCE SUMMARY"
log "============================================================"

echo "PASS : ${PASS}" | tee -a "${LOG_FILE}"
echo "FAIL : ${FAIL}" | tee -a "${LOG_FILE}"
echo "SKIP : ${SKIP}" | tee -a "${LOG_FILE}"

log ""
log "UART Device          : ${UART_DEVICE}"
log "Baud Rate            : ${BAUDRATE}"
log "Format               : 8N1"
log "Bytes/Transfer       : ${BYTES_PER_TRANSFER}"
log "Transfer Count       : ${TRANSFER_COUNT}"
log "Successful Transfers : ${SUCCESSFUL_TRANSFERS}"
log "Failed Transfers     : ${FAILED_TRANSFERS}"
log "Total Bytes          : ${TOTAL_BYTES}"
log "Elapsed Time         : ${ELAPSED_MS} ms"
log "Transfer Rate        : ${TRANSFERS_PER_SEC} transfers/sec"
log "Measured Throughput  : ${BYTES_PER_SEC} bytes/sec"
log "Measured Throughput  : ${KB_PER_SEC} KB/sec"
log "Avg Transfer Latency : ${AVG_LATENCY_US:-N/A} us"
log "Theoretical Rate     : ${THEORETICAL_BYTES_SEC} bytes/sec"

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
