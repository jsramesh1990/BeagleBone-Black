#!/bin/bash

###############################################################################
# BeagleBone Black - GPIO Monitor
#
# File:
#   tools/gpio_monitor.sh
#
# Purpose:
#   Monitor GPIO input/output state continuously and display GPIO changes,
#   direction, value and interrupt activity.
#
# Usage:
#   sudo ./gpio_monitor.sh <gpiochip> <line>
#
# Example:
#   sudo ./gpio_monitor.sh /dev/gpiochip0 20
#
# Optional:
#   INTERVAL=0.1 sudo ./gpio_monitor.sh /dev/gpiochip0 20
#
# NOTE:
#   GPIO line numbers are GPIO-chip offsets, NOT always the physical
#   BeagleBone Black header pin numbers.
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

GPIOCHIP="${1:-/dev/gpiochip0}"
GPIO_LINE="${2:-20}"

INTERVAL="${INTERVAL:-0.5}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs"
LOG_FILE="${LOG_DIR}/gpio_monitor_${TIMESTAMP}.log"

mkdir -p "${LOG_DIR}"

###############################################################################
# Logging
###############################################################################

log()
{
    echo "$1" | tee -a "${LOG_FILE}"
}

###############################################################################
# Cleanup
###############################################################################

cleanup()
{
    log ""
    log "GPIO monitor stopped."
    log "Log file: ${LOG_FILE}"
}

trap cleanup EXIT INT TERM

###############################################################################
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - GPIO Monitor"
log "============================================================"
log "Date       : $(date)"
log "Kernel     : $(uname -r)"
log "GPIO Chip  : ${GPIOCHIP}"
log "GPIO Line  : ${GPIO_LINE}"
log "Interval   : ${INTERVAL} seconds"
log "Log File   : ${LOG_FILE}"
log "============================================================"
log ""

###############################################################################
# Root Check
###############################################################################

if [ "${EUID}" -ne 0 ]; then

    log "[ERROR] Run this script as root."

    log ""
    log "Usage:"
    log "  sudo ./gpio_monitor.sh /dev/gpiochip0 20"

    exit 1
fi

###############################################################################
# GPIO Chip Check
###############################################################################

if [ ! -e "${GPIOCHIP}" ]; then

    log "[ERROR] GPIO chip does not exist: ${GPIOCHIP}"

    log ""
    log "Available GPIO chips:"

    ls -l /dev/gpiochip* 2>/dev/null |
        tee -a "${LOG_FILE}" || true

    exit 1
fi

if [ ! -c "${GPIOCHIP}" ]; then

    log "[ERROR] ${GPIOCHIP} is not a character device."

    exit 1
fi

###############################################################################
# Required Commands
###############################################################################

if ! command -v gpiodetect >/dev/null 2>&1; then

    log "[ERROR] gpiodetect is not installed."

    log "Install libgpiod tools before running this monitor."

    exit 1
fi

if ! command -v gpioinfo >/dev/null 2>&1; then

    log "[ERROR] gpioinfo is not installed."

    exit 1
fi

if ! command -v gpioget >/dev/null 2>&1; then

    log "[ERROR] gpioget is not installed."

    exit 1
fi

###############################################################################
# GPIO Chip Information
###############################################################################

log "------------------------------------------------------------"
log "GPIO CHIP INFORMATION"
log "------------------------------------------------------------"

gpiodetect |
    tee -a "${LOG_FILE}"

log ""

gpioinfo "${GPIOCHIP}" |
    tee -a "${LOG_FILE}"

###############################################################################
# GPIO Line Validation
###############################################################################

log ""
log "------------------------------------------------------------"
log "GPIO LINE VALIDATION"
log "------------------------------------------------------------"

if ! gpioinfo "${GPIOCHIP}" 2>/dev/null |
    grep -qE "line[[:space:]]+${GPIO_LINE}:"; then

    log "[ERROR] GPIO line ${GPIO_LINE} was not found on ${GPIOCHIP}."

    exit 1
fi

log "[PASS] GPIO line ${GPIO_LINE} exists."

###############################################################################
# GPIO Line Information
###############################################################################

log ""
log "------------------------------------------------------------"
log "GPIO LINE STATUS"
log "------------------------------------------------------------"

gpioinfo "${GPIOCHIP}" |
    grep -E "line[[:space:]]+${GPIO_LINE}:" |
    tee -a "${LOG_FILE}"

###############################################################################
# Initial GPIO Value
###############################################################################

log ""
log "------------------------------------------------------------"
log "INITIAL GPIO VALUE"
log "------------------------------------------------------------"

INITIAL_VALUE="$(
    gpioget \
        --numeric \
        "${GPIOCHIP}" \
        "${GPIO_LINE}" \
        2>/dev/null
)"

if [ $? -ne 0 ]; then

    log "[ERROR] Unable to read GPIO ${GPIO_LINE}."

    log ""
    log "Possible causes:"
    log "  - GPIO is already requested by another driver"
    log "  - GPIO is configured as output"
    log "  - Incorrect GPIO chip/line"
    log "  - Device Tree configuration"
    log "  - Pinmux configuration"

    exit 1
fi

log "Initial GPIO value: ${INITIAL_VALUE}"

###############################################################################
# Monitor Loop
###############################################################################

log ""
log "============================================================"
log " GPIO MONITOR STARTED"
log "============================================================"
log ""
log "Monitoring:"
log "  Chip : ${GPIOCHIP}"
log "  Line : ${GPIO_LINE}"
log ""
log "Press Ctrl+C to stop."
log ""

PREVIOUS_VALUE="${INITIAL_VALUE}"

CHANGE_COUNT=0
READ_COUNT=0
ERROR_COUNT=0

START_TIME="$(date +%s)"

while true; do

    CURRENT_TIME="$(date '+%Y-%m-%d %H:%M:%S')"

    VALUE="$(
        gpioget \
            --numeric \
            "${GPIOCHIP}" \
            "${GPIO_LINE}" \
            2>/dev/null
    )"

    STATUS=$?

    READ_COUNT=$((READ_COUNT + 1))

    ###########################################################################
    # Read Error
    ###########################################################################

    if [ "${STATUS}" -ne 0 ]; then

        ERROR_COUNT=$((ERROR_COUNT + 1))

        log "[${CURRENT_TIME}] [ERROR] Unable to read GPIO."

        sleep "${INTERVAL}"

        continue
    fi

    ###########################################################################
    # GPIO State Change
    ###########################################################################

    if [ "${VALUE}" != "${PREVIOUS_VALUE}" ]; then

        CHANGE_COUNT=$((CHANGE_COUNT + 1))

        log "[${CURRENT_TIME}] GPIO ${GPIO_LINE}: ${PREVIOUS_VALUE} -> ${VALUE}"

        if [ "${VALUE}" = "1" ]; then

            log "                         State: HIGH"

        else

            log "                         State: LOW"

        fi

        PREVIOUS_VALUE="${VALUE}"

    else

        log "[${CURRENT_TIME}] GPIO ${GPIO_LINE}: ${VALUE}"

    fi

    ###########################################################################
    # Periodic Statistics
    ###########################################################################

    CURRENT_SECONDS="$(date +%s)"

    ELAPSED=$((CURRENT_SECONDS - START_TIME))

    if [ "${ELAPSED}" -gt 0 ] &&
       [ $((READ_COUNT % 20)) -eq 0 ]; then

        log ""
        log "[STAT] Runtime        : ${ELAPSED} sec"
        log "[STAT] Reads          : ${READ_COUNT}"
        log "[STAT] State changes  : ${CHANGE_COUNT}"
        log "[STAT] Read errors    : ${ERROR_COUNT}"
        log ""

    fi

    sleep "${INTERVAL}"

done
