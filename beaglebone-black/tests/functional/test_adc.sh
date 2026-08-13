#!/bin/bash

###############################################################################
# BeagleBone Black - ADC Functional Test
#
# File:
#   tests/functional/test_adc.sh
#
# Purpose:
#   Verify the AM335x ADC/IIO subsystem and read ADC channels.
#
# Tests:
#   1. IIO subsystem availability
#   2. IIO device detection
#   3. ADC channel detection
#   4. Raw ADC reading
#   5. ADC scale/offset information
#   6. Repeated ADC sampling
#   7. Kernel ADC/IIO messages
#
# Usage:
#   sudo ./tests/functional/test_adc.sh
#   sudo ./tests/functional/test_adc.sh 0
#   sudo ./tests/functional/test_adc.sh 0 10
#
# Examples:
#   Read all available ADC channels:
#       sudo ./tests/functional/test_adc.sh
#
#   Read ADC channel 0:
#       sudo ./tests/functional/test_adc.sh 0
#
#   Read channel 0 ten times:
#       sudo ./tests/functional/test_adc.sh 0 10
#
# IMPORTANT:
#   Do not apply a voltage above the AM335x ADC input limit.
#   Use the BeagleBone Black ADC reference/design limits for your
#   particular board and hardware configuration.
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

CHANNEL="${1:-}"
SAMPLES="${2:-5}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs/functional"
LOG_FILE="${LOG_DIR}/adc_${TIMESTAMP}.log"

mkdir -p "${LOG_DIR}"

###############################################################################
# Colors
###############################################################################

GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[1;33m"
BLUE="\033[0;34m"
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

    fail "This test should be run as root."

    echo
    echo "Use:"
    echo "  sudo ./tests/functional/test_adc.sh"

    exit 1
fi

###############################################################################
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - ADC Functional Test"
log "============================================================"
log "Date       : $(date)"
log "Kernel     : $(uname -r)"
log "Board      : $(tr '\0' '\n' < /proc/device-tree/model 2>/dev/null || echo Unknown)"
log "Channel    : ${CHANNEL:-ALL}"
log "Samples    : ${SAMPLES}"
log "Log File   : ${LOG_FILE}"
log "============================================================"
log ""

###############################################################################
# Validate Sample Count
###############################################################################

if ! [[ "${SAMPLES}" =~ ^[0-9]+$ ]]; then
    fail "Invalid sample count: ${SAMPLES}"
    exit 1
fi

if [ "${SAMPLES}" -lt 1 ]; then
    fail "Sample count must be greater than zero."
    exit 1
fi

###############################################################################
# Test 1 - IIO Subsystem
###############################################################################

log "------------------------------------------------------------"
log "TEST 1: IIO subsystem"
log "------------------------------------------------------------"

if [ -d /sys/bus/iio ]; then
    pass "Linux IIO subsystem is available."
else
    fail "Linux IIO subsystem is not available."
    exit 1
fi

###############################################################################
# Test 2 - IIO Device Detection
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 2: IIO device detection"
log "------------------------------------------------------------"

IIO_DEVICES=$(find /sys/bus/iio/devices \
    -maxdepth 1 \
    -type l \
    -name 'iio:device*' \
    2>/dev/null)

if [ -n "${IIO_DEVICES}" ]; then

    IIO_COUNT=$(echo "${IIO_DEVICES}" | wc -l)

    pass "Detected ${IIO_COUNT} IIO device(s)."

    log ""
    log "IIO devices:"

    echo "${IIO_DEVICES}" | tee -a "${LOG_FILE}"

else

    fail "No IIO devices detected."
    exit 1

fi

###############################################################################
# Test 3 - Identify ADC Devices
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 3: ADC device identification"
log "------------------------------------------------------------"

ADC_DEVICES=""

for DEV in /sys/bus/iio/devices/iio:device*; do

    [ -d "${DEV}" ] || continue

    NAME=""

    if [ -f "${DEV}/name" ]; then
        NAME="$(cat "${DEV}/name" 2>/dev/null)"
    fi

    log "Device: ${DEV}"
    log "Name  : ${NAME:-unknown}"

    CHANNELS=$(find "${DEV}" \
        -maxdepth 1 \
        -name 'in_voltage*_raw' \
        -print 2>/dev/null)

    if [ -n "${CHANNELS}" ]; then
        ADC_DEVICES="${ADC_DEVICES}${DEV}"$'\n'
        pass "ADC channels found in ${DEV}"
    fi

done

if [ -z "${ADC_DEVICES}" ]; then
    fail "No ADC voltage channels found."
    exit 1
fi

###############################################################################
# Function - Read ADC Channel
###############################################################################

read_adc_channel()
{
    DEV="$1"
    CHANNEL_FILE="$2"

    CHANNEL_NAME="$(basename "${CHANNEL_FILE}")"

    log ""
    log "ADC device : ${DEV}"
    log "ADC channel: ${CHANNEL_NAME}"

    if [ ! -r "${CHANNEL_FILE}" ]; then
        fail "Cannot read ${CHANNEL_FILE}"
        return
    fi

    VALUE="$(cat "${CHANNEL_FILE}" 2>/dev/null || true)"

    if [[ "${VALUE}" =~ ^[0-9]+$ ]]; then

        pass "${CHANNEL_NAME} raw value = ${VALUE}"

    else

        fail "Invalid ADC value from ${CHANNEL_NAME}: ${VALUE}"

    fi

    ###########################################################################
    # Scale
    ###########################################################################

    SCALE_FILE="${CHANNEL_FILE%_raw}_scale"

    if [ -f "${SCALE_FILE}" ]; then

        SCALE="$(cat "${SCALE_FILE}" 2>/dev/null || true)"

        log "Scale      : ${SCALE}"

    else

        log "Scale      : unavailable"

    fi

    ###########################################################################
    # Offset
    ###########################################################################

    OFFSET_FILE="${CHANNEL_FILE%_raw}_offset"

    if [ -f "${OFFSET_FILE}" ]; then

        OFFSET="$(cat "${OFFSET_FILE}" 2>/dev/null || true)"

        log "Offset     : ${OFFSET}"

    else

        log "Offset     : unavailable"

    fi

    ###########################################################################
    # Repeated Sampling
    ###########################################################################

    log ""
    log "Sampling ${SAMPLES} times..."

    VALUES=""

    SUM=0
    COUNT=0
    MIN=""
    MAX=""

    for ((i=1; i<=SAMPLES; i++)); do

        VALUE="$(cat "${CHANNEL_FILE}" 2>/dev/null || true)"

        if [[ "${VALUE}" =~ ^[0-9]+$ ]]; then

            log "Sample ${i}: ${VALUE}"

            VALUES="${VALUES}${VALUE}"$'\n'

            SUM=$((SUM + VALUE))
            COUNT=$((COUNT + 1))

            if [ -z "${MIN}" ] || [ "${VALUE}" -lt "${MIN}" ]; then
                MIN="${VALUE}"
            fi

            if [ -z "${MAX}" ] || [ "${VALUE}" -gt "${MAX}" ]; then
                MAX="${VALUE}"
            fi

        else

            fail "ADC read failed on sample ${i}"

        fi

        sleep 0.1

    done

    ###########################################################################
    # Statistics
    ###########################################################################

    if [ "${COUNT}" -gt 0 ]; then

        AVG=$((SUM / COUNT))

        log ""
        log "ADC statistics:"
        log "  Samples : ${COUNT}"
        log "  Minimum : ${MIN}"
        log "  Maximum : ${MAX}"
        log "  Average : ${AVG}"

        pass "Repeated ADC sampling completed."

    else

        fail "No valid ADC samples collected."

    fi
}

###############################################################################
# Test 4 - Read Requested Channel
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 4: ADC channel reading"
log "------------------------------------------------------------"

if [ -n "${CHANNEL}" ]; then

    FOUND=0

    for DEV in /sys/bus/iio/devices/iio:device*; do

        [ -d "${DEV}" ] || continue

        CHANNEL_FILE="${DEV}/in_voltage${CHANNEL}_raw"

        if [ -f "${CHANNEL_FILE}" ]; then

            FOUND=1

            read_adc_channel \
                "${DEV}" \
                "${CHANNEL_FILE}"

            break

        fi

    done

    if [ "${FOUND}" -eq 0 ]; then

        fail "ADC channel ${CHANNEL} was not found."

    fi

else

    log "No specific channel requested."
    log "Reading all available ADC channels."

    FOUND_CHANNEL=0

    for DEV in /sys/bus/iio/devices/iio:device*; do

        [ -d "${DEV}" ] || continue

        for CHANNEL_FILE in \
            "${DEV}"/in_voltage*_raw; do

            [ -f "${CHANNEL_FILE}" ] || continue

            FOUND_CHANNEL=1

            read_adc_channel \
                "${DEV}" \
                "${CHANNEL_FILE}"

        done

    done

    if [ "${FOUND_CHANNEL}" -eq 0 ]; then
        fail "No ADC channels available."
    fi

fi

###############################################################################
# Test 5 - ADC Channel Metadata
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 5: ADC channel metadata"
log "------------------------------------------------------------"

for DEV in /sys/bus/iio/devices/iio:device*; do

    [ -d "${DEV}" ] || continue

    CHANNEL_FILES=$(find "${DEV}" \
        -maxdepth 1 \
        -name 'in_voltage*_raw' \
        -print 2>/dev/null)

    [ -n "${CHANNEL_FILES}" ] || continue

    while read -r RAW_FILE; do

        [ -f "${RAW_FILE}" ] || continue

        BASE="${RAW_FILE%_raw}"

        log ""
        log "Channel: $(basename "${BASE}")"

        if [ -f "${BASE}_scale" ]; then
            log "  scale  = $(cat "${BASE}_scale" 2>/dev/null)"
        fi

        if [ -f "${BASE}_offset" ]; then
            log "  offset = $(cat "${BASE}_offset" 2>/dev/null)"
        fi

        if [ -f "${BASE}_sampling_frequency" ]; then
            log "  sampling_frequency = $(cat "${BASE}_sampling_frequency" 2>/dev/null)"
        fi

    done <<< "${CHANNEL_FILES}"

done

pass "ADC metadata inspection completed."

###############################################################################
# Test 6 - Kernel Logs
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 6: ADC/IIO kernel messages"
log "------------------------------------------------------------"

log "Recent ADC/IIO related messages:"

dmesg 2>/dev/null | \
    grep -iE "adc|iio|ti_am335x_adc|ti_am335x" | \
    tail -50 | \
    tee -a "${LOG_FILE}" || true

pass "ADC/IIO kernel-log inspection completed."

###############################################################################
# Summary
###############################################################################

log ""
log "============================================================"
log " ADC TEST SUMMARY"
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
