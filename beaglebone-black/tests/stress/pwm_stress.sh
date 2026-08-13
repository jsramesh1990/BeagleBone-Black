#!/bin/bash

###############################################################################
# BeagleBone Black - PWM Stress Test
#
# File:
#   tests/stress/pwm_stress.sh
#
# Purpose:
#   Continuously configure and verify PWM frequency and duty cycle for a
#   configurable duration.
#
# Usage:
#   sudo ./pwm_stress.sh
#   sudo ./pwm_stress.sh <pwmchip> <channel> <duration_seconds>
#
# Example:
#   sudo ./pwm_stress.sh 0 0 60
#
# IMPORTANT:
#   Verify the PWM channel is actually routed to the required BeagleBone
#   Black header pin through Device Tree / pinmux configuration.
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

PWMCHIP="${1:-0}"
PWM_CHANNEL="${2:-0}"
DURATION="${3:-60}"

PWM_ROOT="/sys/class/pwm/pwmchip${PWMCHIP}"
PWM_PATH="${PWM_ROOT}/pwm${PWM_CHANNEL}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs/stress"
LOG_FILE="${LOG_DIR}/pwm_stress_${TIMESTAMP}.log"

mkdir -p "${LOG_DIR}"

###############################################################################
# PWM Test Parameters
###############################################################################

# PWM periods in nanoseconds
PERIODS=(
    1000000
    2000000
    5000000
    10000000
)

# Duty cycle percentages
DUTY_CYCLES=(
    10
    25
    50
    75
    90
)

###############################################################################
# Counters
###############################################################################

PASS=0
FAIL=0
SKIP=0

CONFIG_COUNT=0
CONFIG_ERRORS=0
PERIOD_ERRORS=0
DUTY_ERRORS=0
ENABLE_ERRORS=0

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
    if [ -f "${PWM_PATH}/enable" ]; then
        echo 0 > "${PWM_PATH}/enable" 2>/dev/null || true
    fi
}

trap cleanup EXIT INT TERM

###############################################################################
# Root Check
###############################################################################

if [ "${EUID}" -ne 0 ]; then

    fail "Run PWM stress test as root."

    echo
    echo "Usage:"
    echo "  sudo ./pwm_stress.sh"
    echo "  sudo ./pwm_stress.sh 0 0 60"

    exit 1
fi

###############################################################################
# Parameter Validation
###############################################################################

if ! [[ "${PWMCHIP}" =~ ^[0-9]+$ ]]; then

    fail "PWM chip number must be numeric."

    exit 1
fi

if ! [[ "${PWM_CHANNEL}" =~ ^[0-9]+$ ]]; then

    fail "PWM channel must be numeric."

    exit 1
fi

if ! [[ "${DURATION}" =~ ^[0-9]+$ ]] || [ "${DURATION}" -le 0 ]; then

    fail "Duration must be a positive integer."

    exit 1
fi

###############################################################################
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - PWM Stress Test"
log "============================================================"
log "Date          : $(date)"
log "Kernel        : $(uname -r)"
log "PWM Chip      : pwmchip${PWMCHIP}"
log "PWM Channel   : ${PWM_CHANNEL}"
log "Duration      : ${DURATION} seconds"
log "PWM Path      : ${PWM_PATH}"
log "Log File      : ${LOG_FILE}"
log "============================================================"
log ""

###############################################################################
# Test 1 - PWM Subsystem
###############################################################################

log "------------------------------------------------------------"
log "TEST 1: PWM subsystem"
log "------------------------------------------------------------"

if [ -d /sys/class/pwm ]; then

    pass "Linux PWM subsystem is available."

else

    fail "Linux PWM subsystem is unavailable."

    exit 1
fi

###############################################################################
# Test 2 - PWM Controller
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 2: PWM controller"
log "------------------------------------------------------------"

if [ -d "${PWM_ROOT}" ]; then

    pass "${PWM_ROOT} is available."

else

    fail "${PWM_ROOT} does not exist."

    log ""
    log "Available PWM controllers:"

    ls -ld /sys/class/pwm/pwmchip* 2>/dev/null |
        tee -a "${LOG_FILE}" || true

    exit 1
fi

###############################################################################
# Test 3 - PWM Controller Information
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 3: PWM controller information"
log "------------------------------------------------------------"

if [ -f "${PWM_ROOT}/npwm" ]; then

    NUM_CHANNELS="$(cat "${PWM_ROOT}/npwm" 2>/dev/null)"

    log "Number of PWM channels: ${NUM_CHANNELS}"

    if [ "${PWM_CHANNEL}" -ge "${NUM_CHANNELS}" ]; then

        fail "PWM channel ${PWM_CHANNEL} is outside controller range."

        exit 1
    fi

    pass "PWM channel is within controller range."

else

    skip "PWM channel count is unavailable."

fi

###############################################################################
# Test 4 - PWM Export
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 4: PWM channel export"
log "------------------------------------------------------------"

if [ ! -d "${PWM_PATH}" ]; then

    if [ -w "${PWM_ROOT}/export" ]; then

        if echo "${PWM_CHANNEL}" > "${PWM_ROOT}/export" 2>/dev/null; then

            sleep 0.2

            pass "PWM channel ${PWM_CHANNEL} exported."

        else

            fail "Unable to export PWM channel ${PWM_CHANNEL}."

            exit 1
        fi

    else

        fail "PWM export interface is unavailable."

        exit 1
    fi

else

    pass "PWM channel ${PWM_CHANNEL} is already exported."

fi

###############################################################################
# Test 5 - PWM Interface
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 5: PWM sysfs interface"
log "------------------------------------------------------------"

REQUIRED_FILES=(
    period
    duty_cycle
    enable
)

INTERFACE_ERROR=0

for file in "${REQUIRED_FILES[@]}"; do

    if [ -f "${PWM_PATH}/${file}" ]; then

        log "Found: ${PWM_PATH}/${file}"

    else

        log "[ERROR] Missing: ${PWM_PATH}/${file}"

        INTERFACE_ERROR=$((INTERFACE_ERROR + 1))

    fi

done

if [ "${INTERFACE_ERROR}" -eq 0 ]; then

    pass "PWM sysfs interface is complete."

else

    fail "PWM sysfs interface is incomplete."

    exit 1
fi

###############################################################################
# Test 6 - Initial PWM Configuration
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 6: Initial PWM configuration"
log "------------------------------------------------------------"

INITIAL_PERIOD="$(cat "${PWM_PATH}/period" 2>/dev/null || echo "unknown")"
INITIAL_DUTY="$(cat "${PWM_PATH}/duty_cycle" 2>/dev/null || echo "unknown")"
INITIAL_ENABLE="$(cat "${PWM_PATH}/enable" 2>/dev/null || echo "unknown")"

log "Initial period    : ${INITIAL_PERIOD}"
log "Initial duty cycle: ${INITIAL_DUTY}"
log "Initial enable    : ${INITIAL_ENABLE}"

pass "Initial PWM configuration captured."

###############################################################################
# Test 7 - PWM Configuration Stress
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 7: PWM configuration stress"
log "------------------------------------------------------------"

log "Starting PWM stress test..."

START_TIME="$(date +%s)"

PERIOD_INDEX=0
DUTY_INDEX=0

while true; do

    CURRENT_TIME="$(date +%s)"

    ELAPSED=$((CURRENT_TIME - START_TIME))

    if [ "${ELAPSED}" -ge "${DURATION}" ]; then
        break
    fi

    PERIOD="${PERIODS[${PERIOD_INDEX}]}"
    DUTY_PERCENT="${DUTY_CYCLES[${DUTY_INDEX}]}"

    ###########################################################################
    # Calculate duty cycle
    ###########################################################################

    DUTY="$(
        awk "BEGIN {
            printf \"%.0f\", (${PERIOD} * ${DUTY_PERCENT}) / 100
        }"
    )"

    ###########################################################################
    # Disable PWM before reconfiguration
    ###########################################################################

    if ! echo 0 > "${PWM_PATH}/enable" 2>/dev/null; then

        ENABLE_ERRORS=$((ENABLE_ERRORS + 1))

    fi

    ###########################################################################
    # Configure period
    ###########################################################################

    if echo "${PERIOD}" > "${PWM_PATH}/period" 2>/dev/null; then

        :

    else

        PERIOD_ERRORS=$((PERIOD_ERRORS + 1))

    fi

    ###########################################################################
    # Configure duty cycle
    ###########################################################################

    if echo "${DUTY}" > "${PWM_PATH}/duty_cycle" 2>/dev/null; then

        :

    else

        DUTY_ERRORS=$((DUTY_ERRORS + 1))

    fi

    ###########################################################################
    # Enable PWM
    ###########################################################################

    if echo 1 > "${PWM_PATH}/enable" 2>/dev/null; then

        :

    else

        ENABLE_ERRORS=$((ENABLE_ERRORS + 1))

    fi

    ###########################################################################
    # Read back configuration
    ###########################################################################

    READ_PERIOD="$(cat "${PWM_PATH}/period" 2>/dev/null || true)"
    READ_DUTY="$(cat "${PWM_PATH}/duty_cycle" 2>/dev/null || true)"

    if [ "${READ_PERIOD}" != "${PERIOD}" ]; then

        PERIOD_ERRORS=$((PERIOD_ERRORS + 1))

    fi

    if [ "${READ_DUTY}" != "${DUTY}" ]; then

        DUTY_ERRORS=$((DUTY_ERRORS + 1))

    fi

    CONFIG_COUNT=$((CONFIG_COUNT + 1))

    ###########################################################################
    # Progress
    ###########################################################################

    if [ $((CONFIG_COUNT % 100)) -eq 0 ]; then

        log "[INFO] Configurations: ${CONFIG_COUNT}"
        log "       Period: ${PERIOD} ns"
        log "       Duty  : ${DUTY_PERCENT}% (${DUTY} ns)"
        log "       Period errors: ${PERIOD_ERRORS}"
        log "       Duty errors  : ${DUTY_ERRORS}"

    fi

    ###########################################################################
    # Change configuration indexes
    ###########################################################################

    PERIOD_INDEX=$((PERIOD_INDEX + 1))

    if [ "${PERIOD_INDEX}" -ge "${#PERIODS[@]}" ]; then
        PERIOD_INDEX=0
    fi

    DUTY_INDEX=$((DUTY_INDEX + 1))

    if [ "${DUTY_INDEX}" -ge "${#DUTY_CYCLES[@]}" ]; then
        DUTY_INDEX=0
    fi

done

END_TIME="$(date +%s)"

ACTUAL_DURATION=$((END_TIME - START_TIME))

if [ "${ACTUAL_DURATION}" -le 0 ]; then
    ACTUAL_DURATION=1
fi

###############################################################################
# Test 8 - Configuration Rate
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 8: PWM configuration rate"
log "------------------------------------------------------------"

CONFIG_RATE="$(
    awk "BEGIN {
        printf \"%.2f\", ${CONFIG_COUNT} / ${ACTUAL_DURATION}
    }"
)"

log "Actual duration    : ${ACTUAL_DURATION} seconds"
log "Configurations     : ${CONFIG_COUNT}"
log "Configuration rate : ${CONFIG_RATE} configurations/sec"

if [ "${CONFIG_COUNT}" -gt 0 ]; then

    pass "PWM configuration stress completed."

else

    fail "No PWM configurations were completed."

fi

###############################################################################
# Test 9 - Final PWM Configuration
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 9: Final PWM configuration"
log "------------------------------------------------------------"

FINAL_PERIOD="$(cat "${PWM_PATH}/period" 2>/dev/null || echo "unknown")"
FINAL_DUTY="$(cat "${PWM_PATH}/duty_cycle" 2>/dev/null || echo "unknown")"
FINAL_ENABLE="$(cat "${PWM_PATH}/enable" 2>/dev/null || echo "unknown")"

log "Final period    : ${FINAL_PERIOD}"
log "Final duty cycle: ${FINAL_DUTY}"
log "Final enable    : ${FINAL_ENABLE}"

if [ "${FINAL_PERIOD}" != "unknown" ] &&
   [ "${FINAL_DUTY}" != "unknown" ]; then

    pass "Final PWM configuration is readable."

else

    fail "Unable to read final PWM configuration."

fi

###############################################################################
# Test 10 - Duty Cycle Validation
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 10: PWM duty-cycle validation"
log "------------------------------------------------------------"

if [ "${FINAL_PERIOD}" != "unknown" ] &&
   [ "${FINAL_DUTY}" != "unknown" ]; then

    if [ "${FINAL_DUTY}" -le "${FINAL_PERIOD}" ]; then

        pass "Duty cycle is within valid period range."

    else

        fail "Duty cycle exceeds PWM period."

    fi

else

    fail "Unable to validate PWM duty cycle."

fi

###############################################################################
# Test 11 - Disable PWM
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 11: PWM disable"
log "------------------------------------------------------------"

if echo 0 > "${PWM_PATH}/enable" 2>/dev/null; then

    FINAL_ENABLE="$(cat "${PWM_PATH}/enable" 2>/dev/null || true)"

    if [ "${FINAL_ENABLE}" = "0" ]; then

        pass "PWM disabled successfully."

    else

        fail "PWM disable state could not be verified."

    fi

else

    fail "Unable to disable PWM."

fi

###############################################################################
# Test 12 - Kernel PWM Error Scan
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 12: PWM kernel error scan"
log "------------------------------------------------------------"

PWM_ERRORS="$(
    dmesg 2>/dev/null |
    grep -iE \
    "pwm|pwmchip|pinctrl|pinmux|ehrpwm|ecap|error|failed" |
    tail -50
)"

if [ -n "${PWM_ERRORS}" ]; then

    log "Recent PWM-related kernel messages:"
    echo "${PWM_ERRORS}" | tee -a "${LOG_FILE}"

else

    log "No PWM-related kernel errors found."

fi

pass "PWM kernel log scan completed."

###############################################################################
# Test 13 - PWM Controller Information
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 13: PWM controller information"
log "------------------------------------------------------------"

log "PWM controller:"
ls -l "${PWM_ROOT}" 2>/dev/null |
    tee -a "${LOG_FILE}" || true

if [ -f "${PWM_ROOT}/npwm" ]; then

    log "Number of PWM channels: $(cat "${PWM_ROOT}/npwm")"

fi

pass "PWM controller information collected."

###############################################################################
# Summary
###############################################################################

log ""
log "============================================================"
log " PWM STRESS TEST SUMMARY"
log "============================================================"

echo "PASS              : ${PASS}" | tee -a "${LOG_FILE}"
echo "FAIL              : ${FAIL}" | tee -a "${LOG_FILE}"
echo "SKIP              : ${SKIP}" | tee -a "${LOG_FILE}"

log ""
log "PWM Controller    : pwmchip${PWMCHIP}"
log "PWM Channel       : ${PWM_CHANNEL}"
log "Duration          : ${ACTUAL_DURATION} sec"
log "Configurations    : ${CONFIG_COUNT}"
log "Configuration Rate: ${CONFIG_RATE} /sec"
log "Period Errors     : ${PERIOD_ERRORS}"
log "Duty Errors       : ${DUTY_ERRORS}"
log "Enable Errors     : ${ENABLE_ERRORS}"
log "Final Period      : ${FINAL_PERIOD}"
log "Final Duty Cycle  : ${FINAL_DUTY}"
log "Final Enable      : ${FINAL_ENABLE}"

log ""
log "Log File:"
log "${LOG_FILE}"

###############################################################################
# Final Result
###############################################################################

if [ "${PERIOD_ERRORS}" -eq 0 ] &&
   [ "${DUTY_ERRORS}" -eq 0 ] &&
   [ "${ENABLE_ERRORS}" -eq 0 ] &&
   [ "${FAIL}" -eq 0 ]; then

    log ""
    log "RESULT: PASS"

    exit 0

else

    log ""
    log "RESULT: FAIL"

    exit 1

fi
