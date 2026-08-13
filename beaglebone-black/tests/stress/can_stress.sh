# `beaglebone-black/tests/stress/can_stress.sh`

```bash
#!/bin/bash

###############################################################################
# BeagleBone Black - CAN Stress Test
#
# File:
#   tests/stress/can_stress.sh
#
# Purpose:
#   Continuously transmit and receive CAN frames for a configurable duration
#   using a CAN loopback setup. The test checks frame transmission,
#   reception, data integrity, errors, and CAN interface state.
#
# Usage:
#   sudo ./can_stress.sh
#   sudo ./can_stress.sh <interface> <duration_seconds> <frame_count>
#
# Example:
#   sudo ./can_stress.sh can0 60 1000
#
# IMPORTANT:
#   For physical CAN testing, use a proper CAN transceiver and termination.
#   Do NOT connect CANH/CANL directly to GPIO/UART pins.
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

CAN_INTERFACE="${1:-can0}"
DURATION="${2:-60}"
FRAME_COUNT="${3:-1000}"

CAN_BITRATE="${CAN_BITRATE:-500000}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs/stress"
LOG_FILE="${LOG_DIR}/can_stress_${TIMESTAMP}.log"

mkdir -p "${LOG_DIR}"

###############################################################################
# Counters
###############################################################################

PASS=0
FAIL=0
SKIP=0

TX_COUNT=0
RX_COUNT=0
RX_ERRORS=0
TX_ERRORS=0
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
# Cleanup
###############################################################################

cleanup()
{
    if [ -n "${CANDUMP_PID:-}" ]; then
        kill "${CANDUMP_PID}" 2>/dev/null || true
        wait "${CANDUMP_PID}" 2>/dev/null || true
    fi

    if [ -n "${CANLOG_FILE:-}" ]; then
        rm -f "${CANLOG_FILE}"
    fi
}

trap cleanup EXIT INT TERM

###############################################################################
# Root Check
###############################################################################

if [ "${EUID}" -ne 0 ]; then

    fail "Run CAN stress test as root."

    echo
    echo "Usage:"
    echo "  sudo ./can_stress.sh"
    echo "  sudo ./can_stress.sh can0 60 1000"

    exit 1
fi

###############################################################################
# Parameter Validation
###############################################################################

if ! [[ "${DURATION}" =~ ^[0-9]+$ ]] || [ "${DURATION}" -le 0 ]; then

    fail "Duration must be a positive integer."

    exit 1
fi

if ! [[ "${FRAME_COUNT}" =~ ^[0-9]+$ ]] || [ "${FRAME_COUNT}" -le 0 ]; then

    fail "Frame count must be a positive integer."

    exit 1
fi

###############################################################################
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - CAN Stress Test"
log "============================================================"
log "Date          : $(date)"
log "Kernel        : $(uname -r)"
log "CAN Interface : ${CAN_INTERFACE}"
log "CAN Bitrate    : ${CAN_BITRATE}"
log "Duration       : ${DURATION} seconds"
log "Frame Count    : ${FRAME_COUNT}"
log "Log File       : ${LOG_FILE}"
log "============================================================"
log ""

###############################################################################
# Test 1 - CAN Utilities
###############################################################################

log "------------------------------------------------------------"
log "TEST 1: CAN utility availability"
log "------------------------------------------------------------"

if command -v ip >/dev/null 2>&1; then

    pass "ip command is available."

else

    fail "ip command is not available."

    exit 1
fi

if command -v cansend >/dev/null 2>&1; then

    pass "cansend is available."

else

    fail "cansend is not installed."

    log "Install can-utils before running this test."

    exit 1
fi

if command -v candump >/dev/null 2>&1; then

    pass "candump is available."

else

    fail "candump is not installed."

    exit 1
fi

###############################################################################
# Test 2 - CAN Interface
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 2: CAN interface verification"
log "------------------------------------------------------------"

if ip link show "${CAN_INTERFACE}" >/dev/null 2>&1; then

    pass "${CAN_INTERFACE} exists."

else

    fail "${CAN_INTERFACE} does not exist."

    log ""
    log "Available CAN interfaces:"

    ip -br link 2>/dev/null |
        grep -E "can[0-9]+" |
        tee -a "${LOG_FILE}" || true

    exit 1
fi

###############################################################################
# Test 3 - CAN Interface Configuration
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 3: CAN interface configuration"
log "------------------------------------------------------------"

CAN_STATE="$(ip -details link show "${CAN_INTERFACE}" 2>/dev/null)"

echo "${CAN_STATE}" | tee -a "${LOG_FILE}"

if echo "${CAN_STATE}" | grep -q "CAN"; then

    pass "CAN interface configuration detected."

else

    fail "Unable to read CAN interface configuration."

fi

###############################################################################
# Test 4 - CAN Interface State
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 4: CAN interface state"
log "------------------------------------------------------------"

if ip link show "${CAN_INTERFACE}" | grep -q "UP"; then

    pass "${CAN_INTERFACE} is UP."

else

    log "${CAN_INTERFACE} is DOWN."
    log "Attempting to configure and bring interface UP."

    ip link set "${CAN_INTERFACE}" down 2>/dev/null || true

    if ip link set "${CAN_INTERFACE}" type can bitrate "${CAN_BITRATE}" \
        2>/dev/null; then

        pass "CAN bitrate configured."

    else

        fail "Unable to configure CAN bitrate."

        exit 1
    fi

    if ip link set "${CAN_INTERFACE}" up 2>/dev/null; then

        pass "${CAN_INTERFACE} brought UP."

    else

        fail "Unable to bring ${CAN_INTERFACE} UP."

        exit 1
    fi
fi

###############################################################################
# Test 5 - CAN Link Configuration
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 5: CAN link configuration"
log "------------------------------------------------------------"

ip -details link show "${CAN_INTERFACE}" |
    tee -a "${LOG_FILE}"

CAN_BITRATE_DETECTED="$(
    ip -details link show "${CAN_INTERFACE}" 2>/dev/null |
    grep -oE "bitrate [0-9]+" |
    head -1 |
    awk '{print $2}'
)"

if [ -n "${CAN_BITRATE_DETECTED}" ]; then

    log "Detected bitrate: ${CAN_BITRATE_DETECTED}"

    pass "CAN bitrate information available."

else

    skip "CAN bitrate could not be determined from link information."

fi

###############################################################################
# Test 6 - Initial CAN Error State
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 6: Initial CAN error state"
log "------------------------------------------------------------"

INITIAL_CAN_STATE="$(
    ip -details -statistics link show "${CAN_INTERFACE}" 2>/dev/null
)"

echo "${INITIAL_CAN_STATE}" | tee -a "${LOG_FILE}"

INITIAL_RX_ERRORS="$(
    echo "${INITIAL_CAN_STATE}" |
    grep -A2 "RX:" |
    head -2 |
    tail -1 |
    awk '{print $3+0}'
)"

INITIAL_TX_ERRORS="$(
    echo "${INITIAL_CAN_STATE}" |
    grep -A2 "TX:" |
    head -2 |
    tail -1 |
    awk '{print $3+0}'
)"

log "Initial RX errors: ${INITIAL_RX_ERRORS:-0}"
log "Initial TX errors: ${INITIAL_TX_ERRORS:-0}"

pass "Initial CAN error state captured."

###############################################################################
# Test 7 - CAN Loopback
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 7: CAN loopback verification"
log "------------------------------------------------------------"

CANLOG_FILE="/tmp/can_stress_${$}.log"

rm -f "${CANLOG_FILE}"

candump "${CAN_INTERFACE}" > "${CANLOG_FILE}" 2>&1 &
CANDUMP_PID=$!

sleep 1

TEST_ID="123"
TEST_DATA="1122334455667788"

if cansend "${CAN_INTERFACE}" "${TEST_ID}#${TEST_DATA}"; then

    pass "CAN test frame transmitted."

else

    fail "CAN test frame transmission failed."

    exit 1
fi

sleep 1

if grep -q "${TEST_ID}" "${CANLOG_FILE}" &&
   grep -qi "${TEST_DATA}" "${CANLOG_FILE}"; then

    pass "CAN loopback frame received correctly."

else

    fail "CAN loopback frame was not received."

    log ""
    log "Received CAN data:"
    cat "${CANLOG_FILE}" 2>/dev/null |
        tee -a "${LOG_FILE}" || true

    log ""
    log "Check CAN transceiver, termination, wiring, or loopback configuration."

    exit 1
fi

kill "${CANDUMP_PID}" 2>/dev/null || true
wait "${CANDUMP_PID}" 2>/dev/null || true

CANDUMP_PID=""

###############################################################################
# Test 8 - CAN Stress Transmission
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 8: CAN continuous frame stress"
log "------------------------------------------------------------"

CANLOG_FILE="/tmp/can_stress_${$}.log"

rm -f "${CANLOG_FILE}"

candump "${CAN_INTERFACE}" > "${CANLOG_FILE}" 2>&1 &
CANDUMP_PID=$!

sleep 1

START_TIME="$(date +%s)"

for ((i=1; i<=FRAME_COUNT; i++)); do

    BYTE1=$((i % 256))
    BYTE2=$(((i / 256) % 256))
    BYTE3=$(((i / 65536) % 256))
    BYTE4=$((i % 256))

    PAYLOAD="$(printf '%02X%02X%02X%02X' \
        "${BYTE1}" \
        "${BYTE2}" \
        "${BYTE3}" \
        "${BYTE4}")"

    if cansend "${CAN_INTERFACE}" "123#${PAYLOAD}" \
        >/dev/null 2>&1; then

        TX_COUNT=$((TX_COUNT + 1))

    else

        TX_ERRORS=$((TX_ERRORS + 1))

    fi

    CURRENT_TIME="$(date +%s)"

    ELAPSED=$((CURRENT_TIME - START_TIME))

    if [ "${ELAPSED}" -ge "${DURATION}" ]; then
        break
    fi

done

END_TIME="$(date +%s)"

ACTUAL_DURATION=$((END_TIME - START_TIME))

if [ "${ACTUAL_DURATION}" -le 0 ]; then
    ACTUAL_DURATION=1
fi

###############################################################################
# Stop Receiver
###############################################################################

sleep 1

kill "${CANDUMP_PID}" 2>/dev/null || true
wait "${CANDUMP_PID}" 2>/dev/null || true

CANDUMP_PID=""

###############################################################################
# Count Received Frames
###############################################################################

if [ -f "${CANLOG_FILE}" ]; then

    RX_COUNT="$(
        grep -E "${CAN_INTERFACE}.*123" "${CANLOG_FILE}" |
        wc -l
    )"

else

    RX_COUNT=0

fi

###############################################################################
# Data Integrity Check
###############################################################################

if [ "${RX_COUNT}" -gt "${TX_COUNT}" ]; then

    RX_COUNT="${TX_COUNT}"

fi

if [ "${RX_COUNT}" -lt "${TX_COUNT}" ]; then

    DATA_ERRORS=$((TX_COUNT - RX_COUNT))

fi

###############################################################################
# Throughput Calculation
###############################################################################

FRAME_RATE="$(
    awk "BEGIN {
        printf \"%.2f\", ${TX_COUNT} / ${ACTUAL_DURATION}
    }"
)"

###############################################################################
# Test 9 - Frame Count Verification
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 9: CAN frame count verification"
log "------------------------------------------------------------"

log "Frames transmitted : ${TX_COUNT}"
log "Frames received    : ${RX_COUNT}"
log "TX errors          : ${TX_ERRORS}"
log "RX errors          : ${RX_ERRORS}"
log "Missing frames     : ${DATA_ERRORS}"

if [ "${TX_COUNT}" -eq 0 ]; then

    fail "No CAN frames were transmitted."

elif [ "${TX_ERRORS}" -eq 0 ]; then

    pass "CAN frame transmission completed."

else

    fail "CAN transmission errors detected."

fi

if [ "${RX_COUNT}" -gt 0 ]; then

    pass "CAN frames were received."

else

    fail "No CAN frames were received."

fi

###############################################################################
# Test 10 - CAN Error Counters
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 10: CAN error counters"
log "------------------------------------------------------------"

FINAL_CAN_STATE="$(
    ip -details -statistics link show "${CAN_INTERFACE}" 2>/dev/null
)"

echo "${FINAL_CAN_STATE}" | tee -a "${LOG_FILE}"

FINAL_RX_ERRORS="$(
    echo "${FINAL_CAN_STATE}" |
    grep -A2 "RX:" |
    head -2 |
    tail -1 |
    awk '{print $3+0}'
)"

FINAL_TX_ERRORS="$(
    echo "${FINAL_CAN_STATE}" |
    grep -A2 "TX:" |
    head -2 |
    tail -1 |
    awk '{print $3+0}'
)"

log "Final RX errors: ${FINAL_RX_ERRORS:-0}"
log "Final TX errors: ${FINAL_TX_ERRORS:-0}"

###############################################################################
# Test 11 - CAN Interface State After Stress
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 11: CAN interface post-stress state"
log "------------------------------------------------------------"

POST_STATE="$(ip link show "${CAN_INTERFACE}" 2>/dev/null)"

echo "${POST_STATE}" | tee -a "${LOG_FILE}"

if echo "${POST_STATE}" | grep -q "UP"; then

    pass "CAN interface remains UP after stress test."

else

    fail "CAN interface went DOWN after stress test."

fi

###############################################################################
# Test 12 - Kernel CAN Errors
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 12: CAN kernel error scan"
log "------------------------------------------------------------"

CAN_KERNEL_ERRORS="$(
    dmesg 2>/dev/null |
    grep -iE "can|m_can|can0|can1|socketcan|bus-off|bit-error|"
    "frame-error|stuff-error|crc-error|ack-error" |
    tail -50
)"

if [ -n "${CAN_KERNEL_ERRORS}" ]; then

    log "Recent CAN-related kernel messages:"
    echo "${CAN_KERNEL_ERRORS}" | tee -a "${LOG_FILE}"

else

    log "No CAN-related kernel error messages found."

fi

pass "CAN kernel log scan completed."

###############################################################################
# Test 13 - CAN Statistics
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 13: CAN interface statistics"
log "------------------------------------------------------------"

ip -statistics link show "${CAN_INTERFACE}" |
    tee -a "${LOG_FILE}"

pass "CAN interface statistics collected."

###############################################################################
# Summary
###############################################################################

log ""
log "============================================================"
log " CAN STRESS TEST SUMMARY"
log "============================================================"

echo "PASS              : ${PASS}" | tee -a "${LOG_FILE}"
echo "FAIL              : ${FAIL}" | tee -a "${LOG_FILE}"
echo "SKIP              : ${SKIP}" | tee -a "${LOG_FILE}"

log ""
log "CAN Interface     : ${CAN_INTERFACE}"
log "CAN Bitrate       : ${CAN_BITRATE}"
log "Duration          : ${ACTUAL_DURATION} sec"
log "TX Frames         : ${TX_COUNT}"
log "RX Frames         : ${RX_COUNT}"
log "TX Errors         : ${TX_ERRORS}"
log "RX Errors         : ${RX_ERRORS}"
log "Missing Frames    : ${DATA_ERRORS}"
log "Frame Rate        : ${FRAME_RATE} frames/sec"
log "Initial RX Errors : ${INITIAL_RX_ERRORS:-0}"
log "Initial TX Errors : ${INITIAL_TX_ERRORS:-0}"
log "Final RX Errors   : ${FINAL_RX_ERRORS:-0}"
log "Final TX Errors   : ${FINAL_TX_ERRORS:-0}"

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
```

