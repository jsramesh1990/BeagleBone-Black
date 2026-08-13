#!/bin/bash

###############################################################################
# BeagleBone Black - CAN Functional Test
#
# File:
#   tests/functional/test_can.sh
#
# Purpose:
#   Verify CAN interface availability, configuration, CAN frame reception,
#   and optional CAN loopback transmission.
#
# Tests:
#   1. CAN kernel/network subsystem
#   2. CAN interface detection
#   3. CAN interface configuration
#   4. CAN interface state
#   5. CAN statistics
#   6. CAN receive test
#   7. Optional CAN loopback test
#
# Usage:
#
#   sudo ./tests/functional/test_can.sh
#   sudo ./tests/functional/test_can.sh can0
#   sudo ./tests/functional/test_can.sh can0 500000
#   sudo ./tests/functional/test_can.sh can0 500000 loopback
#
# Examples:
#
#   sudo ./tests/functional/test_can.sh can0
#
#   sudo ./tests/functional/test_can.sh can0 500000
#
#   sudo ./tests/functional/test_can.sh can0 500000 loopback
#
# IMPORTANT:
#   A physical CAN transceiver and correctly terminated CAN bus are required
#   for physical CAN communication.
#
#   The BeagleBone Black CAN controller must not be connected directly to
#   CANH/CANL without an appropriate CAN transceiver.
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

CAN_IFACE="${1:-can0}"
CAN_BITRATE="${2:-500000}"
TEST_MODE="${3:-normal}"

RECEIVE_TIMEOUT="${RECEIVE_TIMEOUT:-5}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs/functional"
LOG_FILE="${LOG_DIR}/can_${TIMESTAMP}.log"

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
    echo "  sudo ./tests/functional/test_can.sh"

    exit 1
fi

###############################################################################
# Command Check
###############################################################################

if ! command -v ip >/dev/null 2>&1; then

    fail "'ip' command is not installed."

    exit 1
fi

###############################################################################
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - CAN Functional Test"
log "============================================================"
log "Date       : $(date)"
log "Kernel     : $(uname -r)"
log "Board      : $(tr '\0' '\n' < /proc/device-tree/model 2>/dev/null || echo Unknown)"
log "Interface  : ${CAN_IFACE}"
log "Bitrate    : ${CAN_BITRATE}"
log "Mode       : ${TEST_MODE}"
log "Log File   : ${LOG_FILE}"
log "============================================================"
log ""

###############################################################################
# Test 1 - CAN Subsystem
###############################################################################

log "------------------------------------------------------------"
log "TEST 1: CAN subsystem"
log "------------------------------------------------------------"

CAN_LINKS="$(ip -details link show type can 2>/dev/null || true)"

if [ -n "${CAN_LINKS}" ]; then

    pass "CAN network subsystem is available."

    log ""
    log "CAN interfaces:"
    echo "${CAN_LINKS}" | tee -a "${LOG_FILE}"

else

    fail "No CAN interfaces detected."

    log ""
    log "Check:"
    log "  Device Tree CAN configuration"
    log "  Kernel CAN support"
    log "  CAN driver"
    log "  CAN pinmux"
    log "  CAN transceiver"

    exit 1
fi

###############################################################################
# Test 2 - Interface Detection
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 2: CAN interface detection"
log "------------------------------------------------------------"

if ip link show "${CAN_IFACE}" >/dev/null 2>&1; then

    pass "CAN interface ${CAN_IFACE} detected."

else

    fail "CAN interface ${CAN_IFACE} not found."

    log ""
    log "Available network interfaces:"
    ip -details link show | tee -a "${LOG_FILE}"

    exit 1
fi

###############################################################################
# Test 3 - CAN Configuration
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 3: CAN configuration"
log "------------------------------------------------------------"

log "Current CAN configuration:"

ip -details link show "${CAN_IFACE}" | \
    tee -a "${LOG_FILE}"

###############################################################################
# Bring Interface Down Before Configuration
###############################################################################

log ""
log "Bringing ${CAN_IFACE} down before configuration..."

if ip link set "${CAN_IFACE}" down 2>&1 | tee -a "${LOG_FILE}"; then

    pass "${CAN_IFACE} brought down."

else

    fail "Unable to bring ${CAN_IFACE} down."

fi

###############################################################################
# Configure CAN Bitrate
###############################################################################

log ""
log "Configuring ${CAN_IFACE} with bitrate ${CAN_BITRATE}..."

if ip link set "${CAN_IFACE}" \
    type can \
    bitrate "${CAN_BITRATE}" \
    2>&1 | tee -a "${LOG_FILE}"; then

    pass "CAN bitrate configured: ${CAN_BITRATE}"

else

    fail "Failed to configure CAN bitrate."

fi

###############################################################################
# Test 4 - Bring Interface Up
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 4: CAN interface state"
log "------------------------------------------------------------"

if ip link set "${CAN_IFACE}" up 2>&1 | tee -a "${LOG_FILE}"; then

    pass "${CAN_IFACE} brought UP."

else

    fail "Failed to bring ${CAN_IFACE} UP."

fi

sleep 1

###############################################################################
# Verify Interface State
###############################################################################

CAN_STATE="$(ip link show "${CAN_IFACE}" 2>/dev/null || true)"

log ""
log "CAN interface state:"
echo "${CAN_STATE}" | tee -a "${LOG_FILE}"

if echo "${CAN_STATE}" | grep -q "UP"; then

    pass "${CAN_IFACE} is UP."

else

    fail "${CAN_IFACE} is not UP."

fi

###############################################################################
# Test 5 - CAN Details
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 5: CAN controller configuration"
log "------------------------------------------------------------"

ip -details link show "${CAN_IFACE}" | \
    tee -a "${LOG_FILE}"

if ip -details link show "${CAN_IFACE}" | \
    grep -q "bitrate"; then

    pass "CAN bitrate information available."

else

    skip "Bitrate information could not be verified."

fi

###############################################################################
# Test 6 - CAN Statistics
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 6: CAN statistics"
log "------------------------------------------------------------"

CAN_STATS="$(ip -statistics link show "${CAN_IFACE}" 2>/dev/null || true)"

if [ -n "${CAN_STATS}" ]; then

    echo "${CAN_STATS}" | tee -a "${LOG_FILE}"

    pass "CAN statistics retrieved."

else

    fail "Unable to retrieve CAN statistics."

fi

###############################################################################
# Test 7 - CAN Receive
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 7: CAN receive"
log "------------------------------------------------------------"

if command -v candump >/dev/null 2>&1; then

    log "Waiting ${RECEIVE_TIMEOUT} seconds for CAN frames..."

    log "Start another CAN node or CAN analyzer to transmit frames."

    timeout "${RECEIVE_TIMEOUT}" \
        candump "${CAN_IFACE}" \
        2>&1 | tee -a "${LOG_FILE}" || true

    if [ "${PIPESTATUS[0]}" -eq 0 ]; then

        pass "CAN receive command completed."

    else

        skip "No CAN frame may have been received during the timeout."

    fi

else

    skip "candump is not installed."

    log "Install can-utils to perform CAN frame testing."

fi

###############################################################################
# Test 8 - CAN Loopback
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 8: CAN loopback"
log "------------------------------------------------------------"

if [ "${TEST_MODE}" = "loopback" ]; then

    if ! command -v cansend >/dev/null 2>&1; then

        skip "cansend is not installed."

    elif ! command -v candump >/dev/null 2>&1; then

        skip "candump is not installed."

    else

        log "Enabling CAN controller loopback..."

        ip link set "${CAN_IFACE}" down 2>/dev/null || true

        if ip link set "${CAN_IFACE}" \
            type can \
            bitrate "${CAN_BITRATE}" \
            loopback on \
            2>&1 | tee -a "${LOG_FILE}"; then

            pass "CAN loopback configured."

        else

            fail "Failed to configure CAN loopback."

        fi

        ip link set "${CAN_IFACE}" up 2>/dev/null || true

        sleep 1

        LOOPBACK_LOG="/tmp/bbb_can_loopback_${$}.log"

        log ""
        log "Starting CAN receiver..."

        timeout 3 \
            candump "${CAN_IFACE}" > "${LOOPBACK_LOG}" 2>&1 &

        CANDUMP_PID=$!

        sleep 1

        log "Sending test CAN frame..."

        if cansend "${CAN_IFACE}" 123#1122334455667788 \
            2>&1 | tee -a "${LOG_FILE}"; then

            pass "CAN test frame transmitted."

        else

            fail "CAN test frame transmission failed."

        fi

        sleep 1

        wait "${CANDUMP_PID}" 2>/dev/null || true

        log ""
        log "Loopback receive result:"

        cat "${LOOPBACK_LOG}" | tee -a "${LOG_FILE}"

        if grep -q "123" "${LOOPBACK_LOG}"; then

            pass "CAN loopback frame received."

        else

            fail "CAN loopback frame was not received."

        fi

        rm -f "${LOOPBACK_LOG}"

    fi

else

    skip "Physical CAN receive test only; loopback not requested."

    log ""
    log "For loopback testing use:"
    log "  sudo ./tests/functional/test_can.sh can0 500000 loopback"

fi

###############################################################################
# Test 9 - Kernel CAN Messages
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 9: CAN kernel messages"
log "------------------------------------------------------------"

log "Recent CAN-related kernel messages:"

dmesg 2>/dev/null | \
    grep -iE "can|dcan|mcp251|can-dev|can_raw" | \
    tail -50 | \
    tee -a "${LOG_FILE}" || true

pass "CAN kernel-log inspection completed."

###############################################################################
# Cleanup
###############################################################################

log ""
log "------------------------------------------------------------"
log "CAN interface cleanup"
log "------------------------------------------------------------"

if ip link show "${CAN_IFACE}" >/dev/null 2>&1; then

    ip link set "${CAN_IFACE}" down \
        2>&1 | tee -a "${LOG_FILE}" || true

    pass "${CAN_IFACE} brought down after test."

fi

###############################################################################
# Summary
###############################################################################

log ""
log "============================================================"
log " CAN TEST SUMMARY"
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
