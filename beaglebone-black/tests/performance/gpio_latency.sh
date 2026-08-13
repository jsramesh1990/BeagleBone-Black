#!/bin/bash

###############################################################################
# BeagleBone Black - GPIO Latency Performance Test
#
# File:
#   tests/performance/gpio_latency.sh
#
# Purpose:
#   Measure GPIO read latency and estimate GPIO access performance.
#
# Usage:
#   sudo ./gpio_latency.sh
#   sudo ./gpio_latency.sh <gpio_number> <sample_count>
#
# Example:
#   sudo ./gpio_latency.sh 60 10000
#
# Parameters:
#   gpio_number  - GPIO number to test
#   sample_count - Number of GPIO reads
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

GPIO="${1:-60}"
SAMPLE_COUNT="${2:-10000}"

GPIO_PATH="/sys/class/gpio/gpio${GPIO}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs/performance"
LOG_FILE="${LOG_DIR}/gpio_latency_${TIMESTAMP}.log"

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
    echo "  sudo ./gpio_latency.sh"

    exit 1
fi

###############################################################################
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - GPIO Latency Performance Test"
log "============================================================"
log "Date         : $(date)"
log "Kernel       : $(uname -r)"
log "GPIO         : ${GPIO}"
log "Sample Count : ${SAMPLE_COUNT}"
log "GPIO Path    : ${GPIO_PATH}"
log "Log File     : ${LOG_FILE}"
log "============================================================"
log ""

###############################################################################
# Test 1 - GPIO Subsystem
###############################################################################

log "------------------------------------------------------------"
log "TEST 1: GPIO subsystem"
log "------------------------------------------------------------"

if [ -d /sys/class/gpio ]; then

    pass "GPIO sysfs interface is available."

else

    fail "GPIO sysfs interface is not available."

    exit 1
fi

###############################################################################
# Test 2 - GPIO Export
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 2: GPIO availability"
log "------------------------------------------------------------"

if [ ! -d "${GPIO_PATH}" ]; then

    log "Exporting GPIO${GPIO}..."

    if echo "${GPIO}" > /sys/class/gpio/export 2>/dev/null; then

        sleep 1

    else

        fail "Unable to export GPIO${GPIO}."

    fi

fi

if [ -d "${GPIO_PATH}" ]; then

    pass "GPIO${GPIO} is available."

else

    fail "GPIO${GPIO} is not available."

    exit 1
fi

###############################################################################
# Test 3 - GPIO Input Configuration
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 3: GPIO configuration"
log "------------------------------------------------------------"

if echo in > "${GPIO_PATH}/direction" 2>/dev/null; then

    pass "GPIO${GPIO} configured as input."

else

    fail "Unable to configure GPIO${GPIO} as input."

    exit 1
fi

###############################################################################
# Test 4 - Initial GPIO Read
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 4: Initial GPIO read"
log "------------------------------------------------------------"

INITIAL_VALUE="$(cat "${GPIO_PATH}/value" 2>/dev/null || true)"

if [ "${INITIAL_VALUE}" = "0" ] || [ "${INITIAL_VALUE}" = "1" ]; then

    log "Initial GPIO value: ${INITIAL_VALUE}"

    pass "GPIO value read successfully."

else

    fail "Invalid GPIO value: ${INITIAL_VALUE}"

    exit 1
fi

###############################################################################
# Test 5 - GPIO Read Latency
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 5: GPIO read latency"
log "------------------------------------------------------------"

START_TIME="$(date +%s%N)"

SUCCESSFUL_READS=0
FAILED_READS=0

for ((i=1; i<=SAMPLE_COUNT; i++)); do

    VALUE="$(cat "${GPIO_PATH}/value" 2>/dev/null || true)"

    if [ "${VALUE}" = "0" ] || [ "${VALUE}" = "1" ]; then

        SUCCESSFUL_READS=$((SUCCESSFUL_READS + 1))

    else

        FAILED_READS=$((FAILED_READS + 1))

    fi

done

END_TIME="$(date +%s%N)"

###############################################################################
# Calculate Latency
###############################################################################

ELAPSED_NS=$((END_TIME - START_TIME))

if [ "${SUCCESSFUL_READS}" -gt 0 ]; then

    AVG_LATENCY_NS=$((ELAPSED_NS / SUCCESSFUL_READS))

else

    AVG_LATENCY_NS=0

fi

ELAPSED_US=$((ELAPSED_NS / 1000))

if [ "${ELAPSED_US}" -gt 0 ]; then

    READ_RATE=$((SUCCESSFUL_READS * 1000000 / ELAPSED_US))

else

    READ_RATE=0

fi

###############################################################################
# Results
###############################################################################

log ""
log "GPIO Latency Results"
log "--------------------"
log "Requested reads : ${SAMPLE_COUNT}"
log "Successful reads: ${SUCCESSFUL_READS}"
log "Failed reads    : ${FAILED_READS}"
log "Total time      : ${ELAPSED_NS} ns"
log "Total time      : ${ELAPSED_US} us"
log "Average latency : ${AVG_LATENCY_NS} ns"
log "Read rate       : ${READ_RATE} reads/sec"

if [ "${SUCCESSFUL_READS}" -eq "${SAMPLE_COUNT}" ]; then

    pass "All GPIO reads completed successfully."

else

    fail "Some GPIO reads failed."

fi

###############################################################################
# Test 6 - GPIO Value Stability
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 6: GPIO value stability"
log "------------------------------------------------------------"

ZERO_COUNT=0
ONE_COUNT=0

for ((i=1; i<=100; i++)); do

    VALUE="$(cat "${GPIO_PATH}/value" 2>/dev/null || true)"

    if [ "${VALUE}" = "0" ]; then

        ZERO_COUNT=$((ZERO_COUNT + 1))

    elif [ "${VALUE}" = "1" ]; then

        ONE_COUNT=$((ONE_COUNT + 1))

    fi

done

log "LOW samples : ${ZERO_COUNT}"
log "HIGH samples: ${ONE_COUNT}"

if [ $((ZERO_COUNT + ONE_COUNT)) -eq 100 ]; then

    pass "GPIO returned valid values for all stability samples."

else

    fail "Invalid GPIO values detected."

fi

###############################################################################
# Test 7 - GPIO Information
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 7: GPIO information"
log "------------------------------------------------------------"

log "GPIO number : ${GPIO}"
log "Direction   : $(cat "${GPIO_PATH}/direction" 2>/dev/null)"
log "Value       : $(cat "${GPIO_PATH}/value" 2>/dev/null)"

if [ -f "${GPIO_PATH}/active_low" ]; then

    log "Active low  : $(cat "${GPIO_PATH}/active_low")"

fi

pass "GPIO information inspected."

###############################################################################
# Test 8 - Kernel GPIO Messages
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 8: Kernel GPIO messages"
log "------------------------------------------------------------"

dmesg 2>/dev/null | \
    grep -iE "gpio|gpiochip" | \
    tail -30 | \
    tee -a "${LOG_FILE}" || true

pass "Kernel GPIO log inspection completed."

###############################################################################
# Cleanup
###############################################################################

if [ -d "${GPIO_PATH}" ]; then

    echo "${GPIO}" > /sys/class/gpio/unexport 2>/dev/null || true

fi

###############################################################################
# Summary
###############################################################################

log ""
log "============================================================"
log " GPIO LATENCY PERFORMANCE SUMMARY"
log "============================================================"

echo "PASS : ${PASS}" | tee -a "${LOG_FILE}"
echo "FAIL : ${FAIL}" | tee -a "${LOG_FILE}"
echo "SKIP : ${SKIP}" | tee -a "${LOG_FILE}"

log ""
log "GPIO Number     : ${GPIO}"
log "Sample Count    : ${SAMPLE_COUNT}"
log "Successful Reads: ${SUCCESSFUL_READS}"
log "Failed Reads    : ${FAILED_READS}"
log "Average Latency : ${AVG_LATENCY_NS} ns"
log "Read Rate       : ${READ_RATE} reads/sec"

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
