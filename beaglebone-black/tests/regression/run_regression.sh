#!/bin/bash

###############################################################################
# BeagleBone Black - Complete Regression Test Runner
#
# File:
#   tests/regression/run_regression.sh
#
# Purpose:
#   Execute all BeagleBone Black regression tests and generate a combined
#   regression report.
#
# Usage:
#   sudo ./run_regression.sh
#
# Optional:
#   sudo ./run_regression.sh --stop-on-fail
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs/regression"
LOG_FILE="${LOG_DIR}/run_regression_${TIMESTAMP}.log"

STOP_ON_FAIL=0

if [ "${1:-}" = "--stop-on-fail" ]; then
    STOP_ON_FAIL=1
fi

mkdir -p "${LOG_DIR}"

###############################################################################
# Counters
###############################################################################

TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
SKIPPED_TESTS=0

###############################################################################
# Logging
###############################################################################

log()
{
    echo "$1" | tee -a "${LOG_FILE}"
}

###############################################################################
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - Regression Test Suite"
log "============================================================"
log "Date        : $(date)"
log "Kernel      : $(uname -r)"
log "Architecture: $(uname -m)"
log "Project Root: ${PROJECT_ROOT}"
log "Log File    : ${LOG_FILE}"
log "============================================================"
log ""

###############################################################################
# Root Check
###############################################################################

if [ "${EUID}" -ne 0 ]; then

    log "[FAIL] Regression tests must be executed as root."

    log ""
    log "Use:"
    log "  sudo ./run_regression.sh"

    exit 1
fi

###############################################################################
# Test Execution Function
###############################################################################

run_test()
{
    local test_name="$1"
    local test_script="$2"

    TOTAL_TESTS=$((TOTAL_TESTS + 1))

    log ""
    log "============================================================"
    log " TEST ${TOTAL_TESTS}: ${test_name}"
    log "============================================================"
    log "Script: ${test_script}"
    log ""

    if [ ! -f "${test_script}" ]; then

        log "[SKIP] Test script not found: ${test_script}"

        SKIPPED_TESTS=$((SKIPPED_TESTS + 1))

        return 0
    fi

    if [ ! -x "${test_script}" ]; then

        log "[INFO] Making test executable:"
        log "${test_script}"

        chmod +x "${test_script}"
    fi

    log "[INFO] Starting ${test_name}..."

    if "${test_script}" 2>&1 | tee -a "${LOG_FILE}"; then

        log ""
        log "[PASS] ${test_name}"

        PASSED_TESTS=$((PASSED_TESTS + 1))

        return 0

    else

        log ""
        log "[FAIL] ${test_name}"

        FAILED_TESTS=$((FAILED_TESTS + 1))

        if [ "${STOP_ON_FAIL}" -eq 1 ]; then

            log ""
            log "[INFO] --stop-on-fail enabled."
            log "[INFO] Stopping regression execution."

            return 1

        fi

        return 0

    fi
}

###############################################################################
# Test List
###############################################################################

DEVICE_TREE_TEST="${SCRIPT_DIR}/device_tree_regression.sh"
DRIVER_TEST="${SCRIPT_DIR}/driver_regression.sh"

###############################################################################
# Regression Test 1 - Device Tree
###############################################################################

run_test \
    "Device Tree Regression" \
    "${DEVICE_TREE_TEST}"

if [ $? -ne 0 ] && [ "${STOP_ON_FAIL}" -eq 1 ]; then
    exit 1
fi

###############################################################################
# Regression Test 2 - Driver
###############################################################################

run_test \
    "Driver Regression" \
    "${DRIVER_TEST}"

if [ $? -ne 0 ] && [ "${STOP_ON_FAIL}" -eq 1 ]; then
    exit 1
fi

###############################################################################
# Additional Regression Tests
#
# Add future regression scripts here.
#
# Example:
#
# run_test \
#     "Boot Regression" \
#     "${SCRIPT_DIR}/boot_regression.sh"
#
# run_test \
#     "Peripheral Regression" \
#     "${SCRIPT_DIR}/peripheral_regression.sh"
#
###############################################################################

###############################################################################
# Generate System Information
###############################################################################

log ""
log "============================================================"
log " SYSTEM INFORMATION"
log "============================================================"

log "Kernel:"
uname -a | tee -a "${LOG_FILE}"

log ""
log "CPU:"
lscpu 2>/dev/null | \
    grep -E "Architecture|CPU\(s\)|Model name" | \
    tee -a "${LOG_FILE}" || true

log ""
log "Memory:"
free -h 2>/dev/null | tee -a "${LOG_FILE}" || true

log ""
log "Boot Command Line:"
cat /proc/cmdline 2>/dev/null | tee -a "${LOG_FILE}" || true

###############################################################################
# Device Tree Information
###############################################################################

log ""
log "============================================================"
log " DEVICE TREE INFORMATION"
log "============================================================"

if [ -f /sys/firmware/devicetree/base/model ]; then

    log "Model:"
    tr -d '\0' < /sys/firmware/devicetree/base/model | \
        tee -a "${LOG_FILE}"

fi

if [ -f /sys/firmware/devicetree/base/compatible ]; then

    log ""
    log "Compatible:"
    tr '\0' '\n' < /sys/firmware/devicetree/base/compatible | \
        tee -a "${LOG_FILE}"

fi

###############################################################################
# Driver Information
###############################################################################

log ""
log "============================================================"
log " DRIVER INFORMATION"
log "============================================================"

log "Loaded peripheral-related modules:"

lsmod 2>/dev/null |
    grep -iE \
    "gpio|i2c|spi|uart|serial|pwm|can|adc|ti|omap|mcspi|8250" |
    tee -a "${LOG_FILE}" || true

###############################################################################
# Peripheral Summary
###############################################################################

log ""
log "============================================================"
log " PERIPHERAL SUMMARY"
log "============================================================"

log "I2C:"
ls -l /dev/i2c-* 2>/dev/null | tee -a "${LOG_FILE}" || \
    log "  No I2C device nodes found."

log ""
log "SPI:"
ls -l /dev/spidev* 2>/dev/null | tee -a "${LOG_FILE}" || \
    log "  No SPI device nodes found."

log ""
log "UART:"
ls -l /dev/ttyS* /dev/ttyO* /dev/ttyAMA* 2>/dev/null | \
    tee -a "${LOG_FILE}" || \
    log "  No UART device nodes found."

log ""
log "GPIO:"
ls -l /dev/gpiochip* 2>/dev/null | tee -a "${LOG_FILE}" || \
    log "  No GPIO character devices found."

log ""
log "PWM:"
ls -ld /sys/class/pwm/pwmchip* 2>/dev/null | \
    tee -a "${LOG_FILE}" || \
    log "  No PWM controllers found."

log ""
log "CAN:"
ip -details link show type can 2>/dev/null | \
    tee -a "${LOG_FILE}" || \
    log "  No CAN interfaces found."

log ""
log "ADC/IIO:"
ls -l /sys/bus/iio/devices/ 2>/dev/null | \
    tee -a "${LOG_FILE}" || \
    log "  No IIO devices found."

###############################################################################
# Network Summary
###############################################################################

log ""
log "============================================================"
log " NETWORK SUMMARY"
log "============================================================"

ip -br link 2>/dev/null | tee -a "${LOG_FILE}" || true

###############################################################################
# Kernel Error Scan
###############################################################################

log ""
log "============================================================"
log " KERNEL ERROR SCAN"
log "============================================================"

KERNEL_ERRORS="$(
    dmesg 2>/dev/null |
    grep -iE \
    "error|failed|failure|timeout|panic|oops|BUG:" |
    tail -50
)"

if [ -n "${KERNEL_ERRORS}" ]; then

    log "[WARNING] Potential kernel errors detected:"
    echo "${KERNEL_ERRORS}" | tee -a "${LOG_FILE}"

else

    log "[PASS] No obvious kernel errors found."

fi

###############################################################################
# Final Regression Summary
###############################################################################

log ""
log "============================================================"
log " COMPLETE REGRESSION TEST SUMMARY"
log "============================================================"

log "Total Tests   : ${TOTAL_TESTS}"
log "Passed Tests  : ${PASSED_TESTS}"
log "Failed Tests  : ${FAILED_TESTS}"
log "Skipped Tests : ${SKIPPED_TESTS}"

###############################################################################
# Result Percentage
###############################################################################

if [ "${TOTAL_TESTS}" -gt 0 ]; then

    PASS_PERCENT=$(
        awk "BEGIN {
            printf \"%.2f\", (${PASSED_TESTS} / ${TOTAL_TESTS}) * 100
        }"
    )

else

    PASS_PERCENT="0.00"

fi

log ""
log "Pass Percentage: ${PASS_PERCENT}%"

###############################################################################
# Result Table
###############################################################################

log ""
log "------------------------------------------------------------"
log " Test Result"
log "------------------------------------------------------------"

if [ "${FAILED_TESTS}" -eq 0 ]; then

    log "Device Tree Regression : PASS"
    log "Driver Regression      : PASS"

else

    log "Device Tree Regression : Check individual test log"
    log "Driver Regression      : Check individual test log"

fi

###############################################################################
# Log Location
###############################################################################

log ""
log "------------------------------------------------------------"
log " Regression Log"
log "------------------------------------------------------------"

log "${LOG_FILE}"

###############################################################################
# Final Result
###############################################################################

log ""
log "============================================================"

if [ "${FAILED_TESTS}" -eq 0 ]; then

    log " RESULT: REGRESSION PASS"

    log "============================================================"

    exit 0

else

    log " RESULT: REGRESSION FAIL"

    log "============================================================"

    exit 1

fi
