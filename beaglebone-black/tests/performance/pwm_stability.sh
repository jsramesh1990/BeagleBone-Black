#!/bin/bash

###############################################################################
# BeagleBone Black - PWM Stability Performance Test
#
# File:
#   tests/performance/pwm_stability.sh
#
# Purpose:
#   Verify PWM configuration and monitor PWM output stability over time.
#
# Usage:
#   sudo ./pwm_stability.sh
#   sudo ./pwm_stability.sh <chip> <channel> <period_ns> <duty_ns> <duration>
#
# Example:
#   sudo ./pwm_stability.sh 0 0 20000000 10000000 30
#
# Parameters:
#   chip       - PWM chip number
#   channel    - PWM channel number
#   period_ns  - PWM period in nanoseconds
#   duty_ns    - PWM duty cycle in nanoseconds
#   duration   - Test duration in seconds
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

PWM_CHIP="${1:-0}"
PWM_CHANNEL="${2:-0}"
PERIOD_NS="${3:-20000000}"
DUTY_NS="${4:-10000000}"
DURATION="${5:-30}"

PWM_BASE="/sys/class/pwm/pwmchip${PWM_CHIP}"
PWM_PATH="${PWM_BASE}/pwm${PWM_CHANNEL}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs/performance"
LOG_FILE="${LOG_DIR}/pwm_stability_${TIMESTAMP}.log"

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
    echo "  sudo ./pwm_stability.sh"

    exit 1
fi

###############################################################################
# Parameter Validation
###############################################################################

if [ "${DUTY_NS}" -gt "${PERIOD_NS}" ]; then

    fail "Duty cycle must not be greater than PWM period."

    exit 1
fi

if [ "${DURATION}" -le 0 ]; then

    fail "Duration must be greater than zero."

    exit 1
fi

###############################################################################
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - PWM Stability Performance Test"
log "============================================================"
log "Date          : $(date)"
log "Kernel        : $(uname -r)"
log "PWM Chip      : ${PWM_CHIP}"
log "PWM Channel   : ${PWM_CHANNEL}"
log "Period        : ${PERIOD_NS} ns"
log "Duty Cycle    : ${DUTY_NS} ns"
log "Duration      : ${DURATION} sec"
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

    fail "Linux PWM subsystem is not available."

    exit 1
fi

###############################################################################
# Test 2 - PWM Chip Detection
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 2: PWM chip detection"
log "------------------------------------------------------------"

if [ -d "${PWM_BASE}" ]; then

    pass "PWM chip ${PWM_CHIP} detected."

else

    fail "PWM chip ${PWM_CHIP} not detected."

    log ""
    log "Available PWM chips:"

    ls -d /sys/class/pwm/pwmchip* \
        2>/dev/null | tee -a "${LOG_FILE}" || true

    exit 1
fi

###############################################################################
# Test 3 - PWM Channel Export
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 3: PWM channel availability"
log "------------------------------------------------------------"

EXPORTED_BY_TEST=0

if [ ! -d "${PWM_PATH}" ]; then

    log "Exporting PWM channel ${PWM_CHANNEL}..."

    if echo "${PWM_CHANNEL}" > "${PWM_BASE}/export" 2>/dev/null; then

        EXPORTED_BY_TEST=1

        sleep 1

    else

        fail "Unable to export PWM channel ${PWM_CHANNEL}."

        exit 1
    fi

fi

if [ -d "${PWM_PATH}" ]; then

    pass "PWM channel ${PWM_CHANNEL} is available."

else

    fail "PWM channel ${PWM_CHANNEL} is not available."

    exit 1
fi

###############################################################################
# Test 4 - PWM Configuration
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 4: PWM configuration"
log "------------------------------------------------------------"

# Ensure PWM is disabled before changing configuration.

echo 0 > "${PWM_PATH}/enable" 2>/dev/null || true

if echo "${PERIOD_NS}" > "${PWM_PATH}/period" 2>/dev/null; then

    pass "PWM period configured."

else

    fail "Unable to configure PWM period."

fi

if echo "${DUTY_NS}" > "${PWM_PATH}/duty_cycle" 2>/dev/null; then

    pass "PWM duty cycle configured."

else

    fail "Unable to configure PWM duty cycle."

fi

###############################################################################
# Test 5 - Configuration Verification
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 5: PWM configuration verification"
log "------------------------------------------------------------"

ACTUAL_PERIOD="$(cat "${PWM_PATH}/period" 2>/dev/null || true)"
ACTUAL_DUTY="$(cat "${PWM_PATH}/duty_cycle" 2>/dev/null || true)"

log "Configured period : ${ACTUAL_PERIOD} ns"
log "Configured duty   : ${ACTUAL_DUTY} ns"

if [ "${ACTUAL_PERIOD}" = "${PERIOD_NS}" ]; then

    pass "PWM period verified."

else

    fail "PWM period verification failed."

fi

if [ "${ACTUAL_DUTY}" = "${DUTY_NS}" ]; then

    pass "PWM duty cycle verified."

else

    fail "PWM duty cycle verification failed."

fi

###############################################################################
# Test 6 - Enable PWM
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 6: PWM output enable"
log "------------------------------------------------------------"

if echo 1 > "${PWM_PATH}/enable" 2>/dev/null; then

    pass "PWM output enabled."

else

    fail "Unable to enable PWM output."

    exit 1
fi

sleep 1

###############################################################################
# Test 7 - PWM Stability Monitoring
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 7: PWM stability monitoring"
log "------------------------------------------------------------"

START_TIME="$(date +%s)"

CHECK_COUNT=0
STABLE_COUNT=0
UNSTABLE_COUNT=0

while true; do

    CURRENT_TIME="$(date +%s)"

    ELAPSED=$((CURRENT_TIME - START_TIME))

    if [ "${ELAPSED}" -ge "${DURATION}" ]; then

        break

    fi

    CURRENT_PERIOD="$(cat "${PWM_PATH}/period" 2>/dev/null || true)"
    CURRENT_DUTY="$(cat "${PWM_PATH}/duty_cycle" 2>/dev/null || true)"
    CURRENT_ENABLE="$(cat "${PWM_PATH}/enable" 2>/dev/null || true)"

    CHECK_COUNT=$((CHECK_COUNT + 1))

    if [ "${CURRENT_PERIOD}" = "${PERIOD_NS}" ] && \
       [ "${CURRENT_DUTY}" = "${DUTY_NS}" ] && \
       [ "${CURRENT_ENABLE}" = "1" ]; then

        STABLE_COUNT=$((STABLE_COUNT + 1))

    else

        UNSTABLE_COUNT=$((UNSTABLE_COUNT + 1))

        log "[WARNING] PWM configuration changed:"
        log "  Period : ${CURRENT_PERIOD}"
        log "  Duty   : ${CURRENT_DUTY}"
        log "  Enable : ${CURRENT_ENABLE}"

    fi

    sleep 1

done

###############################################################################
# Stability Result
###############################################################################

log ""
log "PWM Stability Results"
log "---------------------"
log "Monitoring duration : ${DURATION} sec"
log "Configuration checks: ${CHECK_COUNT}"
log "Stable checks       : ${STABLE_COUNT}"
log "Unstable checks     : ${UNSTABLE_COUNT}"

if [ "${UNSTABLE_COUNT}" -eq 0 ] && [ "${CHECK_COUNT}" -gt 0 ]; then

    pass "PWM configuration remained stable."

else

    fail "PWM configuration was unstable."

fi

###############################################################################
# Test 8 - Final Configuration
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 8: Final PWM configuration"
log "------------------------------------------------------------"

FINAL_PERIOD="$(cat "${PWM_PATH}/period" 2>/dev/null || true)"
FINAL_DUTY="$(cat "${PWM_PATH}/duty_cycle" 2>/dev/null || true)"
FINAL_ENABLE="$(cat "${PWM_PATH}/enable" 2>/dev/null || true)"

log "Final period : ${FINAL_PERIOD} ns"
log "Final duty   : ${FINAL_DUTY} ns"
log "Final enable : ${FINAL_ENABLE}"

if [ "${FINAL_PERIOD}" = "${PERIOD_NS}" ] && \
   [ "${FINAL_DUTY}" = "${DUTY_NS}" ] && \
   [ "${FINAL_ENABLE}" = "1" ]; then

    pass "Final PWM configuration is correct."

else

    fail "Final PWM configuration is incorrect."

fi

###############################################################################
# Calculate Frequency and Duty Cycle
###############################################################################

log ""
log "------------------------------------------------------------"
log "PWM Electrical Parameters"
log "------------------------------------------------------------"

FREQUENCY_HZ=$(
    awk "BEGIN {
        if (${PERIOD_NS} > 0)
            printf \"%.2f\", 1000000000 / ${PERIOD_NS}
        else
            print \"0\"
    }"
)

DUTY_PERCENT=$(
    awk "BEGIN {
        if (${PERIOD_NS} > 0)
            printf \"%.2f\", (${DUTY_NS} / ${PERIOD_NS}) * 100
        else
            print \"0\"
    }"
)

log "PWM frequency : ${FREQUENCY_HZ} Hz"
log "Duty cycle    : ${DUTY_PERCENT} %"

pass "PWM frequency and duty cycle calculated."

###############################################################################
# Test 9 - Kernel PWM Messages
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 9: PWM kernel messages"
log "------------------------------------------------------------"

dmesg 2>/dev/null | \
    grep -iE "pwm|pwmchip" | \
    tail -30 | \
    tee -a "${LOG_FILE}" || true

pass "PWM kernel log inspection completed."

###############################################################################
# Disable PWM
###############################################################################

log ""
log "Disabling PWM output..."

echo 0 > "${PWM_PATH}/enable" 2>/dev/null || true

###############################################################################
# Cleanup
###############################################################################

if [ "${EXPORTED_BY_TEST}" -eq 1 ]; then

    log "Unexporting PWM channel ${PWM_CHANNEL}..."

    echo "${PWM_CHANNEL}" > "${PWM_BASE}/unexport" 2>/dev/null || true

fi

###############################################################################
# Summary
###############################################################################

log ""
log "============================================================"
log " PWM STABILITY PERFORMANCE SUMMARY"
log "============================================================"

echo "PASS : ${PASS}" | tee -a "${LOG_FILE}"
echo "FAIL : ${FAIL}" | tee -a "${LOG_FILE}"
echo "SKIP : ${SKIP}" | tee -a "${LOG_FILE}"

log ""
log "PWM Chip          : ${PWM_CHIP}"
log "PWM Channel       : ${PWM_CHANNEL}"
log "Period            : ${PERIOD_NS} ns"
log "Duty Cycle        : ${DUTY_NS} ns"
log "Frequency         : ${FREQUENCY_HZ} Hz"
log "Duty Percentage   : ${DUTY_PERCENT} %"
log "Monitoring Time   : ${DURATION} sec"
log "Stability Checks  : ${CHECK_COUNT}"
log "Stable Checks     : ${STABLE_COUNT}"
log "Unstable Checks   : ${UNSTABLE_COUNT}"

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
