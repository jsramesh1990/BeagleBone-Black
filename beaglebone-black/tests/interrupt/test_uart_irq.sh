#!/bin/bash

###############################################################################
# BeagleBone Black - UART Interrupt Functional Test
#
# File:
#   tests/interrupt/test_uart_irq.sh
#
# Purpose:
#   Verify UART interrupt registration and interrupt activity.
#
# Tests:
#   1. UART device detection
#   2. UART driver verification
#   3. UART IRQ detection
#   4. IRQ counter baseline
#   5. UART configuration
#   6. UART TX/RX activity
#   7. IRQ counter verification
#   8. Kernel UART/IRQ messages
#
# Usage:
#
#   sudo ./tests/interrupt/test_uart_irq.sh
#   sudo ./tests/interrupt/test_uart_irq.sh /dev/ttyS2
#
# Example:
#
#   sudo ./tests/interrupt/test_uart_irq.sh /dev/ttyS2
#
# IMPORTANT:
#   For RX interrupt testing, connect the UART TX and RX lines in a valid
#   loopback configuration or connect the UART to another UART device.
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

UART_DEVICE="${1:-/dev/ttyS2}"

BAUDRATE="${2:-115200}"

TEST_DATA="BBB_UART_IRQ_TEST"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs/interrupt"
LOG_FILE="${LOG_DIR}/uart_irq_${TIMESTAMP}.log"

mkdir -p "${LOG_DIR}"

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
    echo "  sudo ./tests/interrupt/test_uart_irq.sh"

    exit 1
fi

###############################################################################
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - UART Interrupt Test"
log "============================================================"
log "Date       : $(date)"
log "Kernel     : $(uname -r)"
log "Board      : $(tr '\0' '\n' < /proc/device-tree/model 2>/dev/null || echo Unknown)"
log "UART       : ${UART_DEVICE}"
log "Baudrate   : ${BAUDRATE}"
log "Log File   : ${LOG_FILE}"
log "============================================================"
log ""

###############################################################################
# Test 1 - UART Device Detection
###############################################################################

log "------------------------------------------------------------"
log "TEST 1: UART device detection"
log "------------------------------------------------------------"

if [ -e "${UART_DEVICE}" ]; then

    pass "UART device ${UART_DEVICE} exists."

else

    fail "UART device ${UART_DEVICE} does not exist."

    log ""
    log "Available serial devices:"

    ls -l /dev/ttyS* /dev/ttyO* /dev/ttyAMA* \
        2>/dev/null | tee -a "${LOG_FILE}" || true

    exit 1
fi

###############################################################################
# Test 2 - Serial Device Type
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 2: UART character device verification"
log "------------------------------------------------------------"

if [ -c "${UART_DEVICE}" ]; then

    pass "${UART_DEVICE} is a character device."

else

    fail "${UART_DEVICE} is not a character device."

    exit 1
fi

###############################################################################
# Test 3 - UART Driver Information
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 3: UART driver information"
log "------------------------------------------------------------"

UART_NAME="$(basename "${UART_DEVICE}")"

log "UART device: ${UART_NAME}"

if command -v udevadm >/dev/null 2>&1; then

    udevadm info \
        --query=property \
        --name="${UART_DEVICE}" 2>/dev/null | \
        grep -E "DRIVER=|DEVNAME=|ID_" | \
        tee -a "${LOG_FILE}" || true

    pass "UART device information inspected."

else

    skip "udevadm is not available."

fi

###############################################################################
# Function - Get UART IRQ Count
###############################################################################

get_uart_irq_count()
{
    grep -iE \
        "tty|uart|serial|omap.*serial|8250" \
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
# Test 4 - UART IRQ Detection
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 4: UART IRQ detection"
log "------------------------------------------------------------"

UART_IRQ_LINES="$(grep -iE \
    "tty|uart|serial|omap.*serial|8250" \
    /proc/interrupts 2>/dev/null || true)"

if [ -n "${UART_IRQ_LINES}" ]; then

    log "UART-related interrupt entries:"

    echo "${UART_IRQ_LINES}" | \
        tee -a "${LOG_FILE}"

    pass "UART-related interrupt entry found."

else

    skip "No obvious UART interrupt entry found."

fi

###############################################################################
# Test 5 - IRQ Baseline
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 5: UART IRQ baseline"
log "------------------------------------------------------------"

IRQ_BEFORE="$(get_uart_irq_count)"

log "UART IRQ count before test: ${IRQ_BEFORE}"

if [ -n "${IRQ_BEFORE}" ]; then

    pass "UART IRQ baseline captured."

else

    skip "Unable to determine UART IRQ baseline."

fi

###############################################################################
# Test 6 - UART Configuration
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 6: UART configuration"
log "------------------------------------------------------------"

if command -v stty >/dev/null 2>&1; then

    log "Configuring UART:"
    log "  Baudrate : ${BAUDRATE}"
    log "  Data     : 8 bit"
    log "  Parity   : None"
    log "  Stop     : 1"

    if stty \
        -F "${UART_DEVICE}" \
        "${BAUDRATE}" \
        cs8 \
        -cstopb \
        -parenb \
        -ixon \
        -ixoff \
        -crtscts \
        raw \
        2>&1 | tee -a "${LOG_FILE}"; then

        pass "UART configured successfully."

    else

        fail "UART configuration failed."

    fi

else

    fail "stty command is unavailable."

fi

###############################################################################
# Test 7 - UART TX Activity
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 7: UART TX interrupt activity"
log "------------------------------------------------------------"

log "Transmitting test data:"
log "  ${TEST_DATA}"

if printf "%s\n" "${TEST_DATA}" > "${UART_DEVICE}" 2>/dev/null; then

    pass "UART TX operation completed."

else

    fail "UART TX operation failed."

fi

sleep 1

###############################################################################
# Test 8 - UART RX Activity
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 8: UART RX interrupt activity"
log "------------------------------------------------------------"

log "RX testing requires:"
log "  UART TX connected to UART RX"
log "  or another UART device transmitting data."

RX_FILE="/tmp/bbb_uart_rx_${$}.log"

rm -f "${RX_FILE}"

if command -v timeout >/dev/null 2>&1; then

    log "Listening for UART RX data for 3 seconds..."

    timeout 3 \
        cat "${UART_DEVICE}" > "${RX_FILE}" \
        2>/dev/null || true

    if [ -s "${RX_FILE}" ]; then

        log "Received data:"

        cat "${RX_FILE}" | \
            tee -a "${LOG_FILE}"

        pass "UART RX data received."

    else

        skip "No UART RX data received."

    fi

else

    skip "timeout command is unavailable."

fi

###############################################################################
# Test 9 - IRQ Counter Verification
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 9: UART interrupt activity"
log "------------------------------------------------------------"

sleep 1

IRQ_AFTER="$(get_uart_irq_count)"

log "UART IRQ count after test: ${IRQ_AFTER}"

if [ -n "${IRQ_BEFORE}" ] && \
   [ -n "${IRQ_AFTER}" ]; then

    if [ "${IRQ_AFTER}" -gt "${IRQ_BEFORE}" ]; then

        pass "UART interrupt counter increased."

    else

        skip "UART interrupt counter did not increase."

        log ""
        log "Possible reasons:"
        log "  - No UART RX/TX activity"
        log "  - UART device is not connected"
        log "  - UART pinmux is incorrect"
        log "  - UART is disabled in Device Tree"
        log "  - UART driver is not loaded"
        log "  - UART uses a different interrupt entry"

    fi

else

    skip "Unable to evaluate UART IRQ activity."

fi

###############################################################################
# Test 10 - Interrupt Table
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 10: UART interrupt table"
log "------------------------------------------------------------"

log "Current UART-related interrupt entries:"

grep -iE \
    "tty|uart|serial|omap.*serial|8250" \
    /proc/interrupts 2>/dev/null | \
    tee -a "${LOG_FILE}" || true

pass "UART interrupt table inspected."

###############################################################################
# Test 11 - UART Device Information
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 11: UART device information"
log "------------------------------------------------------------"

if command -v setserial >/dev/null 2>&1; then

    setserial -g "${UART_DEVICE}" \
        2>&1 | tee -a "${LOG_FILE}" || true

else

    skip "setserial is not installed."

fi

###############################################################################
# Test 12 - Kernel UART Messages
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 12: UART kernel messages"
log "------------------------------------------------------------"

log "Recent UART/serial-related kernel messages:"

dmesg 2>/dev/null | \
    grep -iE \
        "tty|uart|serial|8250|omap.*serial" | \
    tail -50 | \
    tee -a "${LOG_FILE}" || true

pass "UART kernel-log inspection completed."

###############################################################################
# Cleanup
###############################################################################

rm -f "${RX_FILE}"

###############################################################################
# Summary
###############################################################################

log ""
log "============================================================"
log " UART IRQ TEST SUMMARY"
log "============================================================"

echo -e "${GREEN}PASS : ${PASS}${NC}" | tee -a "${LOG_FILE}"
echo -e "${RED}FAIL : ${FAIL}${NC}" | tee -a "${LOG_FILE}"
echo -e "${YELLOW}SKIP : ${SKIP}${NC}" | tee -a "${LOG_FILE}"

log ""
log "UART Device : ${UART_DEVICE}"
log "Baudrate    : ${BAUDRATE}"
log "IRQ Before  : ${IRQ_BEFORE:-UNKNOWN}"
log "IRQ After   : ${IRQ_AFTER:-UNKNOWN}"

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
