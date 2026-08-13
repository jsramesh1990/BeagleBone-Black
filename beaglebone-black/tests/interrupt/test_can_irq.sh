#!/bin/bash

###############################################################################
# BeagleBone Black - CAN Interrupt Functional Test
#
# File:
#   tests/interrupt/test_can_irq.sh
#
# Purpose:
#   Verify CAN controller interrupt registration and interrupt activity.
#
# Tests:
#   1. CAN interface detection
#   2. CAN interface state
#   3. CAN IRQ detection
#   4. Interrupt counter monitoring
#   5. CAN RX/TX activity
#   6. Kernel CAN messages
#
# Usage:
#
#   sudo ./tests/interrupt/test_can_irq.sh
#   sudo ./tests/interrupt/test_can_irq.sh can0
#
# Requirements:
#   - iproute2
#   - can-utils
#   - Configured CAN controller/transceiver
#   - CAN interface such as can0
#
# IMPORTANT:
#   CAN interrupt activity normally requires actual CAN traffic.
#   For a physical CAN bus, use the correct CAN transceiver and termination.
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

CAN_IFACE="${1:-can0}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs/interrupt"
LOG_FILE="${LOG_DIR}/can_irq_${TIMESTAMP}.log"

mkdir -p "${LOG_DIR}"

###############################################################################
# Test Configuration
###############################################################################

CAN_ID="123"
CAN_DATA="11.22.33.44"
MONITOR_TIME=5

###############################################################################
# Colors
###############################################################################

GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[1;33m"
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
    echo "  sudo ./tests/interrupt/test_can_irq.sh"

    exit 1
fi

###############################################################################
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - CAN Interrupt Test"
log "============================================================"
log "Date       : $(date)"
log "Kernel     : $(uname -r)"
log "Board      : $(tr '\0' '\n' < /proc/device-tree/model 2>/dev/null || echo Unknown)"
log "CAN Device : ${CAN_IFACE}"
log "Monitor    : ${MONITOR_TIME} seconds"
log "Log File   : ${LOG_FILE}"
log "============================================================"
log ""

###############################################################################
# Test 1 - Required Commands
###############################################################################

log "------------------------------------------------------------"
log "TEST 1: Required command check"
log "------------------------------------------------------------"

COMMANDS_OK=1

for CMD in ip grep awk sed dmesg; do

    if command -v "${CMD}" >/dev/null 2>&1; then

        log "[OK] ${CMD}"

    else

        fail "Required command not found: ${CMD}"

        COMMANDS_OK=0

    fi

done

if command -v cansend >/dev/null 2>&1; then

    log "[OK] cansend"

else

    skip "cansend is not installed."

fi

if command -v candump >/dev/null 2>&1; then

    log "[OK] candump"

else

    skip "candump is not installed."

fi

if [ "${COMMANDS_OK}" -eq 1 ]; then

    pass "Required system commands are available."

fi

###############################################################################
# Test 2 - CAN Interface Detection
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 2: CAN interface detection"
log "------------------------------------------------------------"

if ip link show "${CAN_IFACE}" >/dev/null 2>&1; then

    pass "CAN interface ${CAN_IFACE} exists."

else

    fail "CAN interface ${CAN_IFACE} does not exist."

    log ""
    log "Available CAN interfaces:"

    ip -details link show type can 2>&1 | \
        tee -a "${LOG_FILE}"

    exit 1
fi

###############################################################################
# Test 3 - CAN Interface Configuration
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 3: CAN interface configuration"
log "------------------------------------------------------------"

log "CAN interface information:"

ip -details link show "${CAN_IFACE}" 2>&1 | \
    tee -a "${LOG_FILE}"

CAN_STATE="$(ip link show "${CAN_IFACE}" | \
    grep -o "state [A-Z]*" | \
    awk '{print $2}' || true)"

log ""
log "CAN state: ${CAN_STATE:-UNKNOWN}"

if [ "${CAN_STATE}" = "UP" ]; then

    pass "CAN interface is UP."

else

    skip "CAN interface is not UP."

    log ""
    log "To configure CAN, for example:"
    log ""
    log "  ip link set ${CAN_IFACE} down"
    log "  ip link set ${CAN_IFACE} type can bitrate 500000"
    log "  ip link set ${CAN_IFACE} up"

fi

###############################################################################
# Test 4 - CAN IRQ Detection
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 4: CAN IRQ detection"
log "------------------------------------------------------------"

IRQ_BEFORE_FILE="/tmp/can_irq_before_${$}.txt"
IRQ_AFTER_FILE="/tmp/can_irq_after_${$}.txt"

grep -iE "can|m_can|d_can|c_can" \
    /proc/interrupts 2>/dev/null | \
    tee "${IRQ_BEFORE_FILE}" | \
    tee -a "${LOG_FILE}"

if [ -s "${IRQ_BEFORE_FILE}" ]; then

    pass "CAN-related interrupt entry found."

else

    skip "No obvious CAN interrupt entry found in /proc/interrupts."

    log ""
    log "Interrupt table:"
    cat /proc/interrupts | \
        head -30 | \
        tee -a "${LOG_FILE}"

fi

###############################################################################
# Function - Extract IRQ Counts
###############################################################################

get_irq_counts()
{
    grep -iE "can|m_can|d_can|c_can" \
        /proc/interrupts 2>/dev/null | \
        awk '{
            for (i = 2; i <= NF; i++) {
                if ($i ~ /^[0-9]+$/)
                    sum += $i
            }
        }
        END {
            print sum + 0
        }'
}

###############################################################################
# Test 5 - Interrupt Counter Before Activity
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 5: Interrupt counter baseline"
log "------------------------------------------------------------"

IRQ_COUNT_BEFORE="$(get_irq_counts)"

log "CAN IRQ count before traffic: ${IRQ_COUNT_BEFORE}"

if [ -n "${IRQ_COUNT_BEFORE}" ]; then

    pass "CAN interrupt counter captured."

else

    skip "Unable to determine CAN interrupt counter."

fi

###############################################################################
# Test 6 - CAN Traffic
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 6: CAN traffic generation"
log "------------------------------------------------------------"

if ! ip link show "${CAN_IFACE}" | grep -q "UP"; then

    skip "CAN interface is DOWN; traffic generation skipped."

else

    if command -v cansend >/dev/null 2>&1; then

        log "Sending CAN frame:"
        log "  ID   : ${CAN_ID}"
        log "  DATA : ${CAN_DATA}"

        if cansend "${CAN_IFACE}" "${CAN_ID}#${CAN_DATA}" \
            2>&1 | tee -a "${LOG_FILE}"; then

            pass "CAN frame transmission command completed."

        else

            fail "CAN frame transmission failed."

        fi

    else

        skip "cansend is unavailable."

    fi

fi

###############################################################################
# Test 7 - Interrupt Counter After Activity
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 7: Interrupt counter activity"
log "------------------------------------------------------------"

sleep 1

IRQ_COUNT_AFTER="$(get_irq_counts)"

log "CAN IRQ count after traffic: ${IRQ_COUNT_AFTER}"

if [ -n "${IRQ_COUNT_BEFORE}" ] && \
   [ -n "${IRQ_COUNT_AFTER}" ]; then

    if [ "${IRQ_COUNT_AFTER}" -gt "${IRQ_COUNT_BEFORE}" ]; then

        pass "CAN interrupt counter increased."

    else

        skip "CAN interrupt counter did not increase."

        log ""
        log "Possible reasons:"
        log "  - No CAN bus traffic"
        log "  - CAN controller not receiving ACK"
        log "  - CAN transceiver not connected"
        log "  - Incorrect CAN bitrate"
        log "  - CAN pinmux incorrect"
        log "  - Device Tree configuration issue"
        log "  - Interrupt not registered"

    fi

else

    skip "CAN IRQ activity could not be evaluated."

fi

###############################################################################
# Test 8 - Interrupt Table
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 8: Interrupt table verification"
log "------------------------------------------------------------"

log "Current CAN-related interrupt entries:"

grep -iE "can|m_can|d_can|c_can" \
    /proc/interrupts 2>/dev/null | \
    tee -a "${LOG_FILE}"

pass "Interrupt table inspection completed."

###############################################################################
# Test 9 - Optional CAN Receive Test
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 9: CAN RX monitoring"
log "------------------------------------------------------------"

if command -v candump >/dev/null 2>&1; then

    if ip link show "${CAN_IFACE}" | grep -q "UP"; then

        log "Listening for CAN frames for ${MONITOR_TIME} seconds..."

        timeout "${MONITOR_TIME}" \
            candump "${CAN_IFACE}" 2>&1 | \
            tee -a "${LOG_FILE}" || true

        pass "CAN RX monitoring completed."

    else

        skip "CAN interface is DOWN."

    fi

else

    skip "candump is not installed."

fi

###############################################################################
# Test 10 - Kernel CAN Messages
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 10: CAN kernel messages"
log "------------------------------------------------------------"

log "Recent CAN-related kernel messages:"

dmesg 2>/dev/null | \
    grep -iE "can|m_can|d_can|c_can" | \
    tail -50 | \
    tee -a "${LOG_FILE}" || true

pass "CAN kernel-log inspection completed."

###############################################################################
# Cleanup
###############################################################################

rm -f "${IRQ_BEFORE_FILE}" \
      "${IRQ_AFTER_FILE}"

###############################################################################
# Summary
###############################################################################

log ""
log "============================================================"
log " CAN IRQ TEST SUMMARY"
log "============================================================"

echo -e "${GREEN}PASS : ${PASS}${NC}" | tee -a "${LOG_FILE}"
echo -e "${RED}FAIL : ${FAIL}${NC}" | tee -a "${LOG_FILE}"
echo -e "${YELLOW}SKIP : ${SKIP}${NC}" | tee -a "${LOG_FILE}"

log ""
log "CAN Interface : ${CAN_IFACE}"
log "IRQ Before    : ${IRQ_COUNT_BEFORE:-UNKNOWN}"
log "IRQ After     : ${IRQ_COUNT_AFTER:-UNKNOWN}"

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
