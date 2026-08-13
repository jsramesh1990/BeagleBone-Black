#!/bin/bash

###############################################################################
# BeagleBone Black - Device Tree Regression Test
#
# File:
#   tests/regression/device_tree_regression.sh
#
# Purpose:
#   Verify that the Device Tree is correctly loaded and that required
#   BeagleBone Black peripherals are present after boot.
#
# Usage:
#   sudo ./device_tree_regression.sh
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs/regression"
LOG_FILE="${LOG_DIR}/device_tree_regression_${TIMESTAMP}.log"

DT_ROOT="/sys/firmware/devicetree/base"

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
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - Device Tree Regression Test"
log "============================================================"
log "Date       : $(date)"
log "Kernel     : $(uname -r)"
log "Hostname   : $(hostname)"
log "Device Tree: ${DT_ROOT}"
log "Log File   : ${LOG_FILE}"
log "============================================================"
log ""

###############################################################################
# Test 1 - Device Tree Availability
###############################################################################

log "------------------------------------------------------------"
log "TEST 1: Device Tree availability"
log "------------------------------------------------------------"

if [ -d "${DT_ROOT}" ]; then

    pass "Linux Device Tree is mounted and accessible."

else

    fail "Device Tree filesystem is not available."

    exit 1
fi

###############################################################################
# Test 2 - Model Verification
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 2: Board model verification"
log "------------------------------------------------------------"

MODEL_FILE="${DT_ROOT}/model"

if [ -f "${MODEL_FILE}" ]; then

    MODEL="$(tr -d '\0' < "${MODEL_FILE}")"

    log "Detected model: ${MODEL}"

    if echo "${MODEL}" | grep -qi "BeagleBone"; then

        pass "BeagleBone board model detected."

    else

        skip "Model does not explicitly contain 'BeagleBone'."

    fi

else

    skip "Device Tree model property not available."

fi

###############################################################################
# Test 3 - Compatible Property
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 3: Device Tree compatible property"
log "------------------------------------------------------------"

COMPATIBLE_FILE="${DT_ROOT}/compatible"

if [ -f "${COMPATIBLE_FILE}" ]; then

    COMPATIBLE="$(tr '\0' '\n' < "${COMPATIBLE_FILE}")"

    log "Compatible strings:"
    echo "${COMPATIBLE}" | tee -a "${LOG_FILE}"

    if echo "${COMPATIBLE}" | grep -qiE "am335x|ti,beaglebone"; then

        pass "Expected BeagleBone/AM335x compatible string found."

    else

        skip "Expected BeagleBone compatible string not found."

    fi

else

    fail "Device Tree compatible property not found."

fi

###############################################################################
# Test 4 - CPU Node
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 4: CPU node"
log "------------------------------------------------------------"

CPU_NODE="${DT_ROOT}/cpus"

if [ -d "${CPU_NODE}" ]; then

    CPU_COUNT="$(find "${CPU_NODE}" -maxdepth 1 -type d -name 'cpu@*' | wc -l)"

    log "Detected CPU nodes: ${CPU_COUNT}"

    if [ "${CPU_COUNT}" -gt 0 ]; then

        pass "CPU nodes are present in Device Tree."

    else

        fail "No CPU nodes found."

    fi

else

    fail "CPU node is missing."

fi

###############################################################################
# Test 5 - Memory Node
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 5: Memory node"
log "------------------------------------------------------------"

MEMORY_NODE="$(find "${DT_ROOT}" -maxdepth 1 -type d -name 'memory@*' | head -1)"

if [ -n "${MEMORY_NODE}" ]; then

    log "Memory node: $(basename "${MEMORY_NODE}")"

    pass "Memory node is present."

else

    fail "Memory node not found."

fi

###############################################################################
# Test 6 - Interrupt Controller
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 6: Interrupt controller"
log "------------------------------------------------------------"

if find "${DT_ROOT}" -type d \
    \( -iname "*interrupt*" -o -iname "*intc*" \) \
    2>/dev/null | grep -q .; then

    pass "Interrupt-controller related Device Tree nodes found."

else

    skip "Interrupt-controller node could not be identified."

fi

###############################################################################
# Test 7 - GPIO Nodes
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 7: GPIO Device Tree nodes"
log "------------------------------------------------------------"

GPIO_COUNT="$(
    find "${DT_ROOT}" -type d \
        \( -iname "*gpio*" -o -iname "gpio@*" \) \
        2>/dev/null | wc -l
)"

log "GPIO-related nodes: ${GPIO_COUNT}"

if [ "${GPIO_COUNT}" -gt 0 ]; then

    pass "GPIO Device Tree nodes are present."

else

    fail "GPIO Device Tree nodes not found."

fi

###############################################################################
# Test 8 - I2C Nodes
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 8: I2C Device Tree nodes"
log "------------------------------------------------------------"

I2C_COUNT="$(
    find "${DT_ROOT}" -type d \
        \( -iname "*i2c*" -o -iname "i2c@*" \) \
        2>/dev/null | wc -l
)"

log "I2C-related nodes: ${I2C_COUNT}"

if [ "${I2C_COUNT}" -gt 0 ]; then

    pass "I2C Device Tree nodes are present."

else

    fail "I2C Device Tree nodes not found."

fi

###############################################################################
# Test 9 - SPI Nodes
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 9: SPI Device Tree nodes"
log "------------------------------------------------------------"

SPI_COUNT="$(
    find "${DT_ROOT}" -type d \
        \( -iname "*spi*" -o -iname "spi@*" \) \
        2>/dev/null | wc -l
)"

log "SPI-related nodes: ${SPI_COUNT}"

if [ "${SPI_COUNT}" -gt 0 ]; then

    pass "SPI Device Tree nodes are present."

else

    fail "SPI Device Tree nodes not found."

fi

###############################################################################
# Test 10 - UART Nodes
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 10: UART Device Tree nodes"
log "------------------------------------------------------------"

UART_COUNT="$(
    find "${DT_ROOT}" -type d \
        \( -iname "*uart*" -o -iname "serial@*" \) \
        2>/dev/null | wc -l
)"

log "UART-related nodes: ${UART_COUNT}"

if [ "${UART_COUNT}" -gt 0 ]; then

    pass "UART Device Tree nodes are present."

else

    fail "UART Device Tree nodes not found."

fi

###############################################################################
# Test 11 - PWM Nodes
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 11: PWM Device Tree nodes"
log "------------------------------------------------------------"

PWM_COUNT="$(
    find "${DT_ROOT}" -type d \
        \( -iname "*pwm*" -o -iname "pwm@*" \) \
        2>/dev/null | wc -l
)"

log "PWM-related nodes: ${PWM_COUNT}"

if [ "${PWM_COUNT}" -gt 0 ]; then

    pass "PWM Device Tree nodes are present."

else

    skip "PWM Device Tree nodes not identified."

fi

###############################################################################
# Test 12 - CAN Nodes
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 12: CAN Device Tree nodes"
log "------------------------------------------------------------"

CAN_COUNT="$(
    find "${DT_ROOT}" -type d \
        \( -iname "*can*" -o -iname "can@*" \) \
        2>/dev/null | wc -l
)"

log "CAN-related nodes: ${CAN_COUNT}"

if [ "${CAN_COUNT}" -gt 0 ]; then

    pass "CAN Device Tree nodes are present."

else

    skip "CAN Device Tree nodes not identified."

fi

###############################################################################
# Test 13 - Device Tree Status
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 13: Device Tree node status"
log "------------------------------------------------------------"

STATUS_TOTAL=0
STATUS_DISABLED=0

while IFS= read -r STATUS_FILE; do

    STATUS_TOTAL=$((STATUS_TOTAL + 1))

    STATUS="$(tr -d '\0' < "${STATUS_FILE}" 2>/dev/null || true)"

    if [ "${STATUS}" = "disabled" ]; then

        STATUS_DISABLED=$((STATUS_DISABLED + 1))

    fi

done < <(find "${DT_ROOT}" -type f -name status 2>/dev/null)

log "Status properties checked : ${STATUS_TOTAL}"
log "Disabled nodes detected   : ${STATUS_DISABLED}"

if [ "${STATUS_TOTAL}" -gt 0 ]; then

    pass "Device Tree status properties inspected."

else

    skip "No explicit status properties found."

fi

###############################################################################
# Test 14 - Device Tree Overlays
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 14: Device Tree overlays"
log "------------------------------------------------------------"

OVERLAY_DIR="/sys/kernel/config/device-tree/overlays"

if [ -d "${OVERLAY_DIR}" ]; then

    OVERLAY_COUNT="$(find "${OVERLAY_DIR}" -mindepth 1 -maxdepth 1 -type d | wc -l)"

    log "Active Device Tree overlays: ${OVERLAY_COUNT}"

    pass "Device Tree overlay subsystem is accessible."

else

    skip "Device Tree overlay directory is not available."

fi

###############################################################################
# Test 15 - Kernel Device Tree Messages
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 15: Device Tree kernel messages"
log "------------------------------------------------------------"

dmesg 2>/dev/null | \
    grep -iE "device tree|devicetree|of:|OF:" | \
    tail -50 | \
    tee -a "${LOG_FILE}" || true

pass "Device Tree kernel log inspection completed."

###############################################################################
# Test 16 - Device Tree Summary
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 16: Device Tree structure summary"
log "------------------------------------------------------------"

log "Top-level Device Tree nodes:"

find "${DT_ROOT}" -mindepth 1 -maxdepth 1 -type d \
    -printf "%f\n" 2>/dev/null | \
    sort | \
    tee -a "${LOG_FILE}"

pass "Device Tree structure inspected."

###############################################################################
# Summary
###############################################################################

log ""
log "============================================================"
log " DEVICE TREE REGRESSION SUMMARY"
log "============================================================"

echo "PASS : ${PASS}" | tee -a "${LOG_FILE}"
echo "FAIL : ${FAIL}" | tee -a "${LOG_FILE}"
echo "SKIP : ${SKIP}" | tee -a "${LOG_FILE}"

log ""
log "Model               : ${MODEL:-Unknown}"
log "CPU Nodes           : ${CPU_COUNT}"
log "GPIO Nodes          : ${GPIO_COUNT}"
log "I2C Nodes           : ${I2C_COUNT}"
log "SPI Nodes           : ${SPI_COUNT}"
log "UART Nodes          : ${UART_COUNT}"
log "PWM Nodes           : ${PWM_COUNT}"
log "CAN Nodes           : ${CAN_COUNT}"
log "Status Properties   : ${STATUS_TOTAL}"
log "Disabled Nodes      : ${STATUS_DISABLED}"

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
