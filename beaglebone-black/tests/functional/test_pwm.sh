#!/bin/bash

###############################################################################
# BeagleBone Black - PWM Functional Test
#
# File:
#   tests/functional/test_pwm.sh
#
# Purpose:
#   Verify Linux PWM subsystem and perform a basic PWM output test.
#
# Tests:
#   1. PWM subsystem availability
#   2. PWM chip detection
#   3. PWM channel detection
#   4. Period configuration
#   5. Duty-cycle configuration
#   6. PWM enable/disable
#   7. PWM sysfs verification
#   8. Kernel PWM messages
#
# Usage:
#
#   sudo ./tests/functional/test_pwm.sh
#   sudo ./tests/functional/test_pwm.sh 0 0
#   sudo ./tests/functional/test_pwm.sh 0 0 1000000 500000
#
# Arguments:
#
#   $1 = PWM chip number
#   $2 = PWM channel
#   $3 = Period in nanoseconds
#   $4 = Duty cycle in nanoseconds
#
# Example:
#
#   sudo ./tests/functional/test_pwm.sh 0 0 1000000 500000
#
# This generates:
#
#   Period      = 1 ms
#   Duty cycle  = 0.5 ms
#   Duty cycle  = 50%
#
# IMPORTANT:
#   Only test a PWM channel that has been correctly configured through
#   Device Tree/pinmux and is safe to drive on the connected hardware.
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

PWM_CHIP="${1:-}"
PWM_CHANNEL="${2:-}"

PERIOD="${3:-1000000}"
DUTY_CYCLE="${4:-500000}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs/functional"
LOG_FILE="${LOG_DIR}/pwm_${TIMESTAMP}.log"

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
    echo "  sudo ./tests/functional/test_pwm.sh"

    exit 1
fi

###############################################################################
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - PWM Functional Test"
log "============================================================"
log "Date       : $(date)"
log "Kernel     : $(uname -r)"
log "Board      : $(tr '\0' '\n' < /proc/device-tree/model 2>/dev/null || echo Unknown)"
log "PWM Chip   : ${PWM_CHIP:-ALL}"
log "PWM Channel: ${PWM_CHANNEL:-ALL}"
log "Period     : ${PERIOD} ns"
log "Duty Cycle : ${DUTY_CYCLE} ns"
log "Log File   : ${LOG_FILE}"
log "============================================================"
log ""

###############################################################################
# Test 1 - PWM Subsystem
###############################################################################

log "------------------------------------------------------------"
log "TEST 1: PWM subsystem"
log "------------------------------------------------------------"

if [ -d /sys/class/pwm ]; then

    pass "PWM subsystem is available."

else

    fail "/sys/class/pwm does not exist."

    exit 1

fi

###############################################################################
# Test 2 - PWM Chip Detection
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 2: PWM chip detection"
log "------------------------------------------------------------"

PWM_CHIPS="$(find /sys/class/pwm \
    -maxdepth 1 \
    -type l \
    -name 'pwmchip*' \
    2>/dev/null || true)"

if [ -n "${PWM_CHIPS}" ]; then

    CHIP_COUNT="$(echo "${PWM_CHIPS}" | wc -l)"

    pass "Detected ${CHIP_COUNT} PWM chip(s)."

    log ""
    log "Available PWM chips:"

    echo "${PWM_CHIPS}" | tee -a "${LOG_FILE}"

else

    fail "No PWM chips detected."

    exit 1

fi

###############################################################################
# Function - Show PWM Chip
###############################################################################

show_pwm_chip()
{
    CHIP="$1"

    log ""
    log "PWM chip: ${CHIP}"

    if [ -f "${CHIP}/npwm" ]; then
        log "Number of PWM channels: $(cat "${CHIP}/npwm")"
    fi

    if [ -f "${CHIP}/device/modalias" ]; then
        log "Modalias: $(cat "${CHIP}/device/modalias" 2>/dev/null)"
    fi

    if [ -f "${CHIP}/device/uevent" ]; then
        log "Device information:"
        cat "${CHIP}/device/uevent" 2>/dev/null | tee -a "${LOG_FILE}"
    fi
}

###############################################################################
# Test 3 - Display PWM Information
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 3: PWM channel information"
log "------------------------------------------------------------"

for CHIP in /sys/class/pwm/pwmchip*; do

    [ -d "${CHIP}" ] || continue

    show_pwm_chip "${CHIP}"

    log ""

    find "${CHIP}" \
        -maxdepth 1 \
        -type d \
        -name 'pwm*' \
        -print 2>/dev/null | \
        tee -a "${LOG_FILE}"

done

pass "PWM channel information inspected."

###############################################################################
# Function - Export PWM
###############################################################################

export_pwm()
{
    CHIP="$1"
    CHANNEL="$2"

    PWM_PATH="/sys/class/pwm/pwmchip${CHIP}/pwm${CHANNEL}"

    if [ -d "${PWM_PATH}" ]; then

        pass "PWM${CHANNEL} is already exported."

        return 0
    fi

    log "Exporting PWM channel ${CHANNEL}..."

    if echo "${CHANNEL}" > \
        "/sys/class/pwm/pwmchip${CHIP}/export" \
        2>/dev/null; then

        sleep 1

        if [ -d "${PWM_PATH}" ]; then

            pass "PWM${CHANNEL} exported successfully."

            return 0

        fi

    fi

    fail "Unable to export PWM${CHANNEL}."

    return 1
}

###############################################################################
# Test 4 - PWM Channel Selection
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 4: PWM channel selection"
log "------------------------------------------------------------"

if [ -z "${PWM_CHIP}" ] || [ -z "${PWM_CHANNEL}" ]; then

    skip "No PWM chip/channel supplied for active PWM test."

    log ""
    log "Available PWM chips:"

    for CHIP in /sys/class/pwm/pwmchip*; do

        [ -d "${CHIP}" ] || continue

        CHIP_NAME="$(basename "${CHIP}")"

        log "  ${CHIP_NAME}"

        if [ -f "${CHIP}/npwm" ]; then
            log "    channels: $(cat "${CHIP}/npwm")"
        fi

    done

else

    PWM_PATH="/sys/class/pwm/pwmchip${PWM_CHIP}/pwm${PWM_CHANNEL}"

    if [ ! -d "/sys/class/pwm/pwmchip${PWM_CHIP}" ]; then

        fail "PWM chip ${PWM_CHIP} does not exist."

    else

        if export_pwm "${PWM_CHIP}" "${PWM_CHANNEL}"; then

            ACTIVE_PWM=1

        else

            ACTIVE_PWM=0

        fi

    fi

fi

###############################################################################
# Test 5 - PWM Period
###############################################################################

if [ "${ACTIVE_PWM:-0}" -eq 1 ]; then

    log ""
    log "------------------------------------------------------------"
    log "TEST 5: PWM period configuration"
    log "------------------------------------------------------------"

    PWM_PATH="/sys/class/pwm/pwmchip${PWM_CHIP}/pwm${PWM_CHANNEL}"

    if [ -f "${PWM_PATH}/enable" ]; then

        CURRENT_ENABLE="$(cat "${PWM_PATH}/enable" 2>/dev/null)"

        if [ "${CURRENT_ENABLE}" = "1" ]; then

            log "PWM currently enabled."
            log "Disabling before configuration."

            echo 0 > "${PWM_PATH}/enable" 2>/dev/null || true

        fi

    fi

    if echo "${PERIOD}" > "${PWM_PATH}/period" 2>/dev/null; then

        pass "PWM period configured: ${PERIOD} ns."

    else

        fail "Failed to configure PWM period."

    fi

    ###########################################################################
    # Test 6 - Duty Cycle
    ###########################################################################

    log ""
    log "------------------------------------------------------------"
    log "TEST 6: PWM duty-cycle configuration"
    log "------------------------------------------------------------"

    if [ "${DUTY_CYCLE}" -le "${PERIOD}" ]; then

        if echo "${DUTY_CYCLE}" > \
            "${PWM_PATH}/duty_cycle" \
            2>/dev/null; then

            pass "PWM duty cycle configured: ${DUTY_CYCLE} ns."

        else

            fail "Failed to configure PWM duty cycle."

        fi

    else

        fail "Duty cycle cannot be greater than period."

    fi

    ###########################################################################
    # Test 7 - Enable PWM
    ###########################################################################

    log ""
    log "------------------------------------------------------------"
    log "TEST 7: PWM enable"
    log "------------------------------------------------------------"

    if echo 1 > "${PWM_PATH}/enable" 2>/dev/null; then

        pass "PWM enabled successfully."

    else

        fail "Failed to enable PWM."

    fi

    sleep 2

    ###########################################################################
    # Verify PWM
    ###########################################################################

    log ""
    log "PWM configuration after enabling:"
    log "  Period     : $(cat "${PWM_PATH}/period" 2>/dev/null)"
    log "  Duty Cycle : $(cat "${PWM_PATH}/duty_cycle" 2>/dev/null)"
    log "  Enable     : $(cat "${PWM_PATH}/enable" 2>/dev/null)"

    ###########################################################################
    # Test 8 - Disable PWM
    ###########################################################################

    log ""
    log "------------------------------------------------------------"
    log "TEST 8: PWM disable"
    log "------------------------------------------------------------"

    if echo 0 > "${PWM_PATH}/enable" 2>/dev/null; then

        pass "PWM disabled successfully."

    else

        fail "Failed to disable PWM."

    fi

fi

###############################################################################
# Test 9 - Kernel Logs
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 9: PWM kernel messages"
log "------------------------------------------------------------"

log "Recent PWM-related kernel messages:"

dmesg 2>/dev/null | \
    grep -iE "pwm|ehrpwm|ecap" | \
    tail -50 | \
    tee -a "${LOG_FILE}" || true

pass "PWM kernel-log inspection completed."

###############################################################################
# Summary
###############################################################################

log ""
log "============================================================"
log " PWM TEST SUMMARY"
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
