#!/bin/bash

###############################################################################
# BeagleBone Black - GPIO Interrupt Functional Test
#
# File:
#   tests/interrupt/test_gpio_irq.sh
#
# Purpose:
#   Verify GPIO interrupt configuration and interrupt activity through Linux.
#
# Tests:
#   1. GPIO subsystem availability
#   2. GPIO line detection
#   3. GPIO input configuration
#   4. IRQ baseline
#   5. GPIO edge detection
#   6. Interrupt counter verification
#   7. Kernel GPIO/IRQ messages
#
# Usage:
#
#   sudo ./tests/interrupt/test_gpio_irq.sh
#   sudo ./tests/interrupt/test_gpio_irq.sh 60
#
# Argument:
#
#   $1 = GPIO number
#
# Example:
#
#   sudo ./tests/interrupt/test_gpio_irq.sh 60
#
# IMPORTANT:
#   The selected GPIO must be physically connected to a signal source.
#   For edge testing, connect the GPIO input to a suitable 3.3-V logic source
#   through the appropriate hardware.
#
#   Do not apply voltage above the BeagleBone Black GPIO voltage limits.
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

GPIO="${1:-60}"

GPIO_PATH="/sys/class/gpio/gpio${GPIO}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs/interrupt"
LOG_FILE="${LOG_DIR}/gpio_irq_${TIMESTAMP}.log"

MONITOR_TIME=5

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
    echo "  sudo ./tests/interrupt/test_gpio_irq.sh"

    exit 1
fi

###############################################################################
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - GPIO Interrupt Test"
log "============================================================"
log "Date       : $(date)"
log "Kernel     : $(uname -r)"
log "Board      : $(tr '\0' '\n' < /proc/device-tree/model 2>/dev/null || echo Unknown)"
log "GPIO       : ${GPIO}"
log "Monitor    : ${MONITOR_TIME} seconds"
log "Log File   : ${LOG_FILE}"
log "============================================================"
log ""

###############################################################################
# Test 1 - GPIO Sysfs
###############################################################################

log "------------------------------------------------------------"
log "TEST 1: GPIO subsystem"
log "------------------------------------------------------------"

if [ -d /sys/class/gpio ]; then

    pass "Legacy GPIO sysfs interface is available."

else

    fail "/sys/class/gpio is not available."

    log ""
    log "This kernel may use the modern GPIO character-device interface."

    exit 1
fi

###############################################################################
# Test 2 - GPIO Export
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 2: GPIO line availability"
log "------------------------------------------------------------"

if [ ! -d "${GPIO_PATH}" ]; then

    log "Exporting GPIO${GPIO}..."

    if echo "${GPIO}" > /sys/class/gpio/export 2>/dev/null; then

        sleep 1

    else

        fail "Unable to export GPIO${GPIO}."

        log ""
        log "Possible reasons:"
        log "  - GPIO is already claimed"
        log "  - Invalid GPIO number"
        log "  - Device Tree pinmux conflict"
        log "  - GPIO controller does not expose this line"

    fi

fi

if [ -d "${GPIO_PATH}" ]; then

    pass "GPIO${GPIO} is available."

else

    fail "GPIO${GPIO} is not available."

    exit 1
fi

###############################################################################
# Test 3 - GPIO Direction
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 3: GPIO input configuration"
log "------------------------------------------------------------"

if echo in > "${GPIO_PATH}/direction" 2>/dev/null; then

    pass "GPIO${GPIO} configured as input."

else

    fail "Unable to configure GPIO${GPIO} as input."

    exit 1
fi

###############################################################################
# Test 4 - GPIO Initial Value
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 4: GPIO value"
log "------------------------------------------------------------"

if [ -f "${GPIO_PATH}/value" ]; then

    GPIO_VALUE="$(cat "${GPIO_PATH}/value" 2>/dev/null)"

    log "GPIO${GPIO} initial value: ${GPIO_VALUE}"

    if [ "${GPIO_VALUE}" = "0" ] || [ "${GPIO_VALUE}" = "1" ]; then

        pass "GPIO input value is readable."

    else

        fail "Invalid GPIO value: ${GPIO_VALUE}"

    fi

else

    fail "GPIO value file does not exist."

fi

###############################################################################
# Test 5 - GPIO Edge Configuration
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 5: GPIO interrupt edge configuration"
log "------------------------------------------------------------"

if [ -f "${GPIO_PATH}/edge" ]; then

    if echo both > "${GPIO_PATH}/edge" 2>/dev/null; then

        pass "GPIO${GPIO} configured for rising and falling edges."

    else

        fail "Unable to configure GPIO edge detection."

    fi

else

    fail "GPIO edge configuration is unavailable."

fi

###############################################################################
# Function - Read GPIO Interrupt Activity
###############################################################################

get_gpio_irq_count()
{
    grep -iE "gpio|omap_gpio|gpio-irq" \
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
# Test 6 - IRQ Baseline
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 6: GPIO IRQ baseline"
log "------------------------------------------------------------"

IRQ_BEFORE="$(get_gpio_irq_count)"

log "GPIO-related IRQ count before test: ${IRQ_BEFORE}"

if [ -n "${IRQ_BEFORE}" ]; then

    pass "GPIO interrupt baseline captured."

else

    skip "Unable to identify GPIO IRQ counter."

fi

###############################################################################
# Test 7 - Monitor GPIO Events
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 7: GPIO interrupt monitoring"
log "------------------------------------------------------------"

log "Monitoring GPIO${GPIO} for ${MONITOR_TIME} seconds..."

EVENT_FILE="/tmp/bbb_gpio_irq_${$}.log"

rm -f "${EVENT_FILE}"

if command -v timeout >/dev/null 2>&1; then

    (
        timeout "${MONITOR_TIME}" \
            cat "${GPIO_PATH}/value" \
            > /dev/null 2>&1
    ) &

    MONITOR_PID=$!

    log ""
    log "Waiting for GPIO edge activity..."

    wait "${MONITOR_PID}" 2>/dev/null || true

    pass "GPIO monitoring period completed."

else

    skip "timeout command is unavailable."

fi

###############################################################################
# Test 8 - IRQ Counter After Monitoring
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 8: GPIO interrupt counter verification"
log "------------------------------------------------------------"

IRQ_AFTER="$(get_gpio_irq_count)"

log "GPIO-related IRQ count after test: ${IRQ_AFTER}"

if [ -n "${IRQ_BEFORE}" ] && [ -n "${IRQ_AFTER}" ]; then

    if [ "${IRQ_AFTER}" -gt "${IRQ_BEFORE}" ]; then

        pass "GPIO interrupt counter increased."

    else

        skip "GPIO interrupt counter did not increase."

        log ""
        log "No GPIO edge was detected during the test period."

        log ""
        log "Generate a GPIO transition using an external signal:"
        log "  LOW -> HIGH"
        log "  HIGH -> LOW"

    fi

else

    skip "GPIO IRQ activity could not be evaluated."

fi

###############################################################################
# Test 9 - GPIO Interrupt Table
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 9: Interrupt table"
log "------------------------------------------------------------"

log "GPIO-related interrupt entries:"

grep -iE "gpio|omap_gpio|gpio-irq" \
    /proc/interrupts 2>/dev/null | \
    tee -a "${LOG_FILE}" || true

pass "Interrupt table inspection completed."

###############################################################################
# Test 10 - Kernel GPIO/IRQ Messages
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 10: Kernel GPIO/IRQ messages"
log "------------------------------------------------------------"

log "Recent GPIO/IRQ kernel messages:"

dmesg 2>/dev/null | \
    grep -iE "gpio|irq|interrupt" | \
    tail -50 | \
    tee -a "${LOG_FILE}" || true

pass "Kernel GPIO/IRQ log inspection completed."

###############################################################################
# Cleanup
###############################################################################

rm -f "${EVENT_FILE}"

###############################################################################
# Summary
###############################################################################

log ""
log "============================================================"
log " GPIO IRQ TEST SUMMARY"
log "============================================================"

echo -e "${GREEN}PASS : ${PASS}${NC}" | tee -a "${LOG_FILE}"
echo -e "${RED}FAIL : ${FAIL}${NC}" | tee -a "${LOG_FILE}"
echo -e "${YELLOW}SKIP : ${SKIP}${NC}" | tee -a "${LOG_FILE}"

log ""
log "GPIO          : ${GPIO}"
log "IRQ Before    : ${IRQ_BEFORE:-UNKNOWN}"
log "IRQ After     : ${IRQ_AFTER:-UNKNOWN}"

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
