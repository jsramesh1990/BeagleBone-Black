#!/bin/bash

###############################################################################
# BeagleBone Black - UART Stress Test
#
# File:
#   tests/stress/uart_stress.sh
#
# Purpose:
#   Continuously transmit and receive UART data for a configurable duration
#   and monitor communication errors, throughput, device availability,
#   and kernel UART errors.
#
# Usage:
#   sudo ./uart_stress.sh
#   sudo ./uart_stress.sh <uart_device> <duration_seconds> <iterations>
#
# Example:
#   sudo ./uart_stress.sh /dev/ttyS1 60 10000
#
# IMPORTANT:
#   This test expects UART loopback:
#
#       TX ---------------- RX
#
#   Connect the UART TX pin to RX pin using the correct voltage level.
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

UART_DEVICE="${1:-/dev/ttyS1}"
DURATION="${2:-60}"
ITERATIONS="${3:-10000}"

BAUD_RATE="${BAUD_RATE:-115200}"
DATA_BITS="${DATA_BITS:-8}"
PARITY="${PARITY:-none}"
STOP_BITS="${STOP_BITS:-1}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs/stress"
LOG_FILE="${LOG_DIR}/uart_stress_${TIMESTAMP}.log"

mkdir -p "${LOG_DIR}"

###############################################################################
# Counters
###############################################################################

PASS=0
FAIL=0
SKIP=0

TX_COUNT=0
RX_COUNT=0
TX_ERRORS=0
RX_ERRORS=0
DATA_ERRORS=0

TOTAL_BYTES_TX=0
TOTAL_BYTES_RX=0

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
    stty sane < "${UART_DEVICE}" 2>/dev/null || true
}

trap cleanup EXIT INT TERM

###############################################################################
# Root Check
###############################################################################

if [ "${EUID}" -ne 0 ]; then

    fail "Run UART stress test as root."

    echo
    echo "Usage:"
    echo "  sudo ./uart_stress.sh"
    echo "  sudo ./uart_stress.sh /dev/ttyS1 60 10000"

    exit 1
fi

###############################################################################
# Parameter Validation
###############################################################################

if [ ! -e "${UART_DEVICE}" ]; then

    fail "UART device does not exist: ${UART_DEVICE}"

    log ""
    log "Available serial devices:"

    ls -l /dev/ttyS* /dev/ttyUSB* /dev/ttyACM* 2>/dev/null |
        tee -a "${LOG_FILE}" || true

    exit 1
fi

if [ ! -c "${UART_DEVICE}" ]; then

    fail "${UART_DEVICE} is not a character device."

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
log " BeagleBone Black - UART Stress Test"
log "============================================================"
log "Date          : $(date)"
log "Kernel        : $(uname -r)"
log "UART Device   : ${UART_DEVICE}"
log "Baud Rate     : ${BAUD_RATE}"
log "Data Bits     : ${DATA_BITS}"
log "Parity        : ${PARITY}"
log "Stop Bits     : ${STOP_BITS}"
log "Duration      : ${DURATION} seconds"
log "Iterations    : ${ITERATIONS}"
log "Log File      : ${LOG_FILE}"
log "============================================================"
log ""

###############################################################################
# Test 1 - UART Device Verification
###############################################################################

log "------------------------------------------------------------"
log "TEST 1: UART device verification"
log "------------------------------------------------------------"

if [ -c "${UART_DEVICE}" ]; then

    ls -l "${UART_DEVICE}" | tee -a "${LOG_FILE}"

    pass "${UART_DEVICE} is available."

else

    fail "UART device is unavailable."

    exit 1
fi

###############################################################################
# Test 2 - UART Configuration
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 2: UART configuration"
log "------------------------------------------------------------"

if stty \
    "${BAUD_RATE}" \
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
    < "${UART_DEVICE}" 2>/dev/null; then

    pass "UART configured successfully."

else

    fail "Unable to configure UART."

    exit 1
fi

###############################################################################
# Test 3 - UART Current Configuration
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 3: UART runtime configuration"
log "------------------------------------------------------------"

stty -a < "${UART_DEVICE}" |
    tee -a "${LOG_FILE}"

pass "UART configuration captured."

###############################################################################
# Test 4 - Basic UART Loopback
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 4: UART basic loopback"
log "------------------------------------------------------------"

TEST_MESSAGE="BBB_UART_LOOPBACK_TEST"

printf "%s" "${TEST_MESSAGE}" > "${UART_DEVICE}"

sleep 0.1

RECEIVED_DATA="$(
    timeout 2 cat "${UART_DEVICE}" 2>/dev/null
)"

if [ "${RECEIVED_DATA}" = "${TEST_MESSAGE}" ]; then

    pass "UART basic loopback test passed."

else

    log "Expected: ${TEST_MESSAGE}"
    log "Received: ${RECEIVED_DATA}"

    fail "UART basic loopback test failed."

    log ""
    log "Check:"
    log "  - TX connected to RX"
    log "  - Correct UART device"
    log "  - Baud rate"
    log "  - UART pinmux"
    log "  - Voltage levels"
    log "  - Device Tree configuration"

    exit 1
fi

###############################################################################
# Test 5 - UART Stress Loop
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 5: UART continuous communication stress"
log "------------------------------------------------------------"

log "Starting UART stress test..."

START_TIME="$(date +%s)"

for ((i=1; i<=ITERATIONS; i++)); do

    CURRENT_TIME="$(date +%s)"

    ELAPSED=$((CURRENT_TIME - START_TIME))

    if [ "${ELAPSED}" -ge "${DURATION}" ]; then
        break
    fi

    ###########################################################################
    # Generate test message
    ###########################################################################

    TEST_MESSAGE="BBB_UART_STRESS_${i}_DATA"

    MESSAGE_LENGTH="${#TEST_MESSAGE}"

    ###########################################################################
    # Transmit
    ###########################################################################

    if printf "%s\n" "${TEST_MESSAGE}" > "${UART_DEVICE}" 2>/dev/null; then

        TX_COUNT=$((TX_COUNT + 1))
        TOTAL_BYTES_TX=$((TOTAL_BYTES_TX + MESSAGE_LENGTH + 1))

    else

        TX_ERRORS=$((TX_ERRORS + 1))

        continue
    fi

    ###########################################################################
    # Receive
    ###########################################################################

    RECEIVED_DATA="$(
        timeout 2 head -c $((MESSAGE_LENGTH + 1)) \
        "${UART_DEVICE}" 2>/dev/null
    )"

    if [ -n "${RECEIVED_DATA}" ]; then

        RX_COUNT=$((RX_COUNT + 1))
        TOTAL_BYTES_RX=$((TOTAL_BYTES_RX + ${#RECEIVED_DATA}))

    else

        RX_ERRORS=$((RX_ERRORS + 1))

        continue
    fi

    ###########################################################################
    # Data verification
    ###########################################################################

    RECEIVED_DATA="${RECEIVED_DATA//$'\n'/}"

    if [ "${RECEIVED_DATA}" != "${TEST_MESSAGE}" ]; then

        DATA_ERRORS=$((DATA_ERRORS + 1))

        if [ "${DATA_ERRORS}" -le 10 ]; then

            log "[ERROR] Iteration ${i}"
            log "        Expected: ${TEST_MESSAGE}"
            log "        Received: ${RECEIVED_DATA}"

        fi

    fi

    ###########################################################################
    # Progress
    ###########################################################################

    if [ $((i % 100)) -eq 0 ]; then

        log "[INFO] Iteration: ${i}"
        log "       TX: ${TX_COUNT}"
        log "       RX: ${RX_COUNT}"
        log "       TX Errors: ${TX_ERRORS}"
        log "       RX Errors: ${RX_ERRORS}"
        log "       Data Errors: ${DATA_ERRORS}"

    fi

done

END_TIME="$(date +%s)"

ACTUAL_DURATION=$((END_TIME - START_TIME))

if [ "${ACTUAL_DURATION}" -le 0 ]; then
    ACTUAL_DURATION=1
fi

###############################################################################
# Test 6 - UART Throughput
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 6: UART throughput"
log "------------------------------------------------------------"

TX_RATE="$(
    awk "BEGIN {
        printf \"%.2f\", ${TOTAL_BYTES_TX} / ${ACTUAL_DURATION}
    }"
)"

RX_RATE="$(
    awk "BEGIN {
        printf \"%.2f\", ${TOTAL_BYTES_RX} / ${ACTUAL_DURATION}
    }"
)"

log "Actual duration : ${ACTUAL_DURATION} seconds"
log "TX operations   : ${TX_COUNT}"
log "RX operations   : ${RX_COUNT}"
log "TX bytes        : ${TOTAL_BYTES_TX}"
log "RX bytes        : ${TOTAL_BYTES_RX}"
log "TX rate         : ${TX_RATE} bytes/sec"
log "RX rate         : ${RX_RATE} bytes/sec"

if [ "${TX_COUNT}" -gt 0 ] &&
   [ "${RX_COUNT}" -gt 0 ]; then

    pass "UART throughput test completed."

else

    fail "UART throughput test failed."

fi

###############################################################################
# Test 7 - UART Error Check
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 7: UART communication error check"
log "------------------------------------------------------------"

if [ "${TX_ERRORS}" -eq 0 ] &&
   [ "${RX_ERRORS}" -eq 0 ] &&
   [ "${DATA_ERRORS}" -eq 0 ]; then

    pass "No UART communication errors detected."

else

    fail "UART communication errors detected."

fi

###############################################################################
# Test 8 - UART Device Recheck
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 8: UART post-stress device check"
log "------------------------------------------------------------"

if [ -c "${UART_DEVICE}" ]; then

    pass "UART device remains available after stress test."

else

    fail "UART device disappeared after stress test."

fi

###############################################################################
# Test 9 - UART Driver Information
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 9: UART driver information"
log "------------------------------------------------------------"

if command -v udevadm >/dev/null 2>&1; then

    udevadm info \
        --query=property \
        --name="${UART_DEVICE}" \
        2>/dev/null |
        grep -E "DRIVER=|DEVNAME=|SUBSYSTEM=" |
        tee -a "${LOG_FILE}" || true

    pass "UART driver information collected."

else

    skip "udevadm is unavailable."

fi

###############################################################################
# Test 10 - UART Kernel Error Scan
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 10: UART kernel error scan"
log "------------------------------------------------------------"

UART_ERRORS="$(
    dmesg 2>/dev/null |
    grep -iE \
    "uart|serial|ttyS|omap|8250|rx|tx|overrun|"
    "framing|parity|break|timeout|error|failed" |
    tail -50
)"

if [ -n "${UART_ERRORS}" ]; then

    log "Recent UART-related kernel messages:"
    echo "${UART_ERRORS}" |
        tee -a "${LOG_FILE}"

else

    log "No UART-related kernel error messages found."

fi

pass "UART kernel log scan completed."

###############################################################################
# Test 11 - UART Device Tree Information
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 11: UART Device Tree information"
log "------------------------------------------------------------"

UART_DT_INFO="$(
    find /sys/firmware/devicetree/base \
        -type d \
        -iname "*serial*" -o \
        -iname "*uart*" \
        2>/dev/null |
    head -20
)"

if [ -n "${UART_DT_INFO}" ]; then

    echo "${UART_DT_INFO}" |
        tee -a "${LOG_FILE}"

    pass "UART Device Tree information collected."

else

    skip "UART Device Tree nodes could not be identified."

fi

###############################################################################
# Test 12 - UART Line Status
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 12: UART line status"
log "------------------------------------------------------------"

if command -v setserial >/dev/null 2>&1; then

    setserial "${UART_DEVICE}" 2>/dev/null |
        tee -a "${LOG_FILE}" || true

    pass "UART line status collected."

else

    skip "setserial is not installed."

fi

###############################################################################
# Summary
###############################################################################

log ""
log "============================================================"
log " UART STRESS TEST SUMMARY"
log "============================================================"

echo "PASS              : ${PASS}" | tee -a "${LOG_FILE}"
echo "FAIL              : ${FAIL}" | tee -a "${LOG_FILE}"
echo "SKIP              : ${SKIP}" | tee -a "${LOG_FILE}"

log ""
log "UART Device       : ${UART_DEVICE}"
log "Baud Rate         : ${BAUD_RATE}"
log "Duration          : ${ACTUAL_DURATION} sec"
log "TX Operations     : ${TX_COUNT}"
log "RX Operations     : ${RX_COUNT}"
log "TX Bytes          : ${TOTAL_BYTES_TX}"
log "RX Bytes          : ${TOTAL_BYTES_RX}"
log "TX Errors         : ${TX_ERRORS}"
log "RX Errors         : ${RX_ERRORS}"
log "Data Errors       : ${DATA_ERRORS}"
log "TX Rate           : ${TX_RATE} bytes/sec"
log "RX Rate           : ${RX_RATE} bytes/sec"

log ""
log "Log File:"
log "${LOG_FILE}"

###############################################################################
# Final Result
###############################################################################

if [ "${TX_ERRORS}" -eq 0 ] &&
   [ "${RX_ERRORS}" -eq 0 ] &&
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
