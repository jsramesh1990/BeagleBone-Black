#!/bin/bash

###############################################################################
# BeagleBone Black - UART Functional Test
#
# File:
#   tests/functional/test_uart.sh
#
# Purpose:
#   Verify Linux UART subsystem, UART device nodes, configuration and
#   optional TX/RX loopback communication.
#
# Tests:
#   1. UART subsystem availability
#   2. UART device detection
#   3. UART sysfs information
#   4. UART configuration
#   5. UART device open/read/write
#   6. Optional TX/RX loopback test
#   7. Kernel UART messages
#
# Usage:
#
#   sudo ./tests/functional/test_uart.sh
#   sudo ./tests/functional/test_uart.sh /dev/ttyS1
#   sudo ./tests/functional/test_uart.sh /dev/ttyS1 115200
#   sudo ./tests/functional/test_uart.sh /dev/ttyS1 115200 loopback
#
# Arguments:
#
#   $1 = UART device
#   $2 = Baud rate
#   $3 = Test mode
#
# Example:
#
#   sudo ./tests/functional/test_uart.sh /dev/ttyS1 115200 loopback
#
# IMPORTANT:
#   Physical loopback requires TX and RX to be connected correctly.
#
#   Do not connect UART pins directly to RS-232 voltage levels.
#   Use the appropriate UART/TTL-to-RS232 or UART/TTL-to-USB transceiver.
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

UART_DEVICE="${1:-}"
BAUDRATE="${2:-115200}"
TEST_MODE="${3:-normal}"

TEST_MESSAGE="BBB_UART_TEST_12345"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs/functional"
LOG_FILE="${LOG_DIR}/uart_${TIMESTAMP}.log"

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

    fail "This script must be run as root."

    echo
    echo "Use:"
    echo "  sudo ./tests/functional/test_uart.sh"

    exit 1
fi

###############################################################################
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - UART Functional Test"
log "============================================================"
log "Date       : $(date)"
log "Kernel     : $(uname -r)"
log "Board      : $(tr '\0' '\n' < /proc/device-tree/model 2>/dev/null || echo Unknown)"
log "UART Device: ${UART_DEVICE:-ALL}"
log "Baud Rate  : ${BAUDRATE}"
log "Mode       : ${TEST_MODE}"
log "Log File   : ${LOG_FILE}"
log "============================================================"
log ""

###############################################################################
# Test 1 - UART Subsystem
###############################################################################

log "------------------------------------------------------------"
log "TEST 1: UART subsystem"
log "------------------------------------------------------------"

if [ -d /sys/class/tty ]; then

    pass "Linux TTY/UART subsystem is available."

else

    fail "Linux TTY subsystem is not available."

    exit 1
fi

###############################################################################
# Test 2 - UART Device Detection
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 2: UART device detection"
log "------------------------------------------------------------"

UART_DEVICES=""

for DEV in /dev/ttyS* /dev/ttyO* /dev/ttyAMA* /dev/ttyUSB* /dev/ttyACM*; do

    if [ -e "${DEV}" ]; then
        UART_DEVICES="${UART_DEVICES}
${DEV}"
    fi

done

if [ -n "${UART_DEVICES}" ]; then

    log "Detected UART/serial devices:"

    echo "${UART_DEVICES}" | \
        sed '/^$/d' | \
        tee -a "${LOG_FILE}"

    pass "UART/serial device(s) detected."

else

    fail "No UART/serial device nodes detected."

fi

###############################################################################
# Test 3 - UART Sysfs Information
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 3: UART sysfs information"
log "------------------------------------------------------------"

if [ -d /sys/class/tty ]; then

    log "Serial devices registered with Linux:"

    for TTY in /sys/class/tty/*; do

        [ -e "${TTY}" ] || continue

        TTY_NAME="$(basename "${TTY}")"

        case "${TTY_NAME}" in

            ttyS*|ttyO*|ttyAMA*|ttyUSB*|ttyACM*)

                log ""
                log "Device: ${TTY_NAME}"

                if [ -L "${TTY}/device" ]; then
                    log "  Device path: $(readlink -f "${TTY}/device")"
                fi

                ;;

        esac

    done

    pass "UART sysfs information inspected."

else

    fail "Unable to access UART sysfs."

fi

###############################################################################
# Test 4 - Selected UART
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 4: Selected UART verification"
log "------------------------------------------------------------"

ACTIVE_UART=0

if [ -z "${UART_DEVICE}" ]; then

    skip "No UART device supplied for active communication test."

    log ""
    log "Example:"
    log "  sudo ./tests/functional/test_uart.sh /dev/ttyS1 115200"

else

    if [ ! -e "${UART_DEVICE}" ]; then

        fail "UART device ${UART_DEVICE} does not exist."

    elif [ ! -c "${UART_DEVICE}" ]; then

        fail "${UART_DEVICE} is not a character device."

    else

        pass "UART device ${UART_DEVICE} exists."

        ACTIVE_UART=1

    fi

fi

###############################################################################
# Test 5 - UART Configuration
###############################################################################

if [ "${ACTIVE_UART}" -eq 1 ]; then

    log ""
    log "------------------------------------------------------------"
    log "TEST 5: UART configuration"
    log "------------------------------------------------------------"

    if ! command -v stty >/dev/null 2>&1; then

        fail "'stty' command is not available."

    else

        log "Current UART configuration:"

        stty -F "${UART_DEVICE}" -a 2>&1 | \
            tee -a "${LOG_FILE}"

        log ""
        log "Configuring UART:"
        log "  Baud rate : ${BAUDRATE}"
        log "  Data bits : 8"
        log "  Parity    : None"
        log "  Stop bits : 1"
        log "  Flow ctrl : None"

        if stty -F "${UART_DEVICE}" \
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

    fi

else

    skip "UART configuration skipped because no UART device was selected."

fi

###############################################################################
# Test 6 - UART Device Open
###############################################################################

if [ "${ACTIVE_UART}" -eq 1 ]; then

    log ""
    log "------------------------------------------------------------"
    log "TEST 6: UART device open"
    log "------------------------------------------------------------"

    exec 3<>"${UART_DEVICE}" 2>/dev/null

    if [ $? -eq 0 ]; then

        pass "UART device opened successfully."

        exec 3>&-

    else

        fail "Unable to open UART device."

    fi

else

    skip "UART open test skipped."

fi

###############################################################################
# Test 7 - UART Loopback
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 7: UART TX/RX loopback"
log "------------------------------------------------------------"

if [ "${ACTIVE_UART}" -eq 1 ]; then

    if [ "${TEST_MODE}" = "loopback" ]; then

        LOOPBACK_FILE="/tmp/bbb_uart_loopback_${$}.log"

        rm -f "${LOOPBACK_FILE}"

        log "Test message:"
        log "  ${TEST_MESSAGE}"

        log ""
        log "Starting UART receiver..."

        (
            timeout 5 cat "${UART_DEVICE}" > "${LOOPBACK_FILE}"
        ) &

        RX_PID=$!

        sleep 1

        log "Transmitting UART test message..."

        if printf '%s\n' "${TEST_MESSAGE}" > \
            "${UART_DEVICE}" 2>/dev/null; then

            pass "UART TX completed."

        else

            fail "UART TX failed."

        fi

        sleep 2

        kill "${RX_PID}" 2>/dev/null || true
        wait "${RX_PID}" 2>/dev/null || true

        log ""
        log "Received data:"

        if [ -f "${LOOPBACK_FILE}" ]; then

            cat "${LOOPBACK_FILE}" | \
                tee -a "${LOG_FILE}"

        fi

        if grep -q "${TEST_MESSAGE}" "${LOOPBACK_FILE}" 2>/dev/null; then

            pass "UART TX/RX loopback successful."

        else

            fail "UART loopback message was not received."

            log ""
            log "Check:"
            log "  TX connected to RX"
            log "  GND connected"
            log "  Baud rate"
            log "  UART pinmux"
            log "  Device Tree configuration"
            log "  UART console conflict"

        fi

        rm -f "${LOOPBACK_FILE}"

    else

        skip "UART loopback not requested."

        log ""
        log "For physical loopback testing use:"
        log ""
        log "  sudo ./tests/functional/test_uart.sh \\"
        log "      ${UART_DEVICE} ${BAUDRATE} loopback"

    fi

else

    skip "UART loopback skipped because no UART device was selected."

fi

###############################################################################
# Test 8 - UART Kernel Messages
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 8: UART kernel messages"
log "------------------------------------------------------------"

log "Recent UART/serial kernel messages:"

dmesg 2>/dev/null | \
    grep -iE "tty|serial|uart|omap" | \
    tail -50 | \
    tee -a "${LOG_FILE}" || true

pass "UART kernel-log inspection completed."

###############################################################################
# Test 9 - UART Device Tree
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 9: UART Device Tree status"
log "------------------------------------------------------------"

if [ -d /proc/device-tree ]; then

    UART_DT_FOUND=0

    for NODE in /proc/device-tree/*uart* \
                /proc/device-tree/*serial* \
                /proc/device-tree/ocp/*uart* \
                /proc/device-tree/ocp/*serial*; do

        if [ -e "${NODE}" ]; then

            log "Device Tree node:"
            log "  ${NODE}"

            UART_DT_FOUND=1

        fi

    done

    if [ "${UART_DT_FOUND}" -eq 1 ]; then

        pass "UART-related Device Tree nodes found."

    else

        skip "No obvious UART Device Tree node found."

    fi

else

    skip "/proc/device-tree is not available."

fi

###############################################################################
# Summary
###############################################################################

log ""
log "============================================================"
log " UART TEST SUMMARY"
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
