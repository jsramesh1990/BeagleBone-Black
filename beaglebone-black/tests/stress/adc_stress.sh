#!/bin/bash

###############################################################################
# BeagleBone Black - ADC Stress Test
#
# File:
#   tests/stress/adc_stress.sh
#
# Purpose:
#   Continuously sample ADC/IIO channels for a configurable duration and
#   monitor ADC availability, sampling rate, invalid readings, and errors.
#
# Usage:
#   sudo ./adc_stress.sh
#   sudo ./adc_stress.sh <duration_seconds> <interval_ms>
#
# Example:
#   sudo ./adc_stress.sh 60 10
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

DURATION="${1:-60}"
INTERVAL_MS="${2:-10}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs/stress"
LOG_FILE="${LOG_DIR}/adc_stress_${TIMESTAMP}.log"

IIO_ROOT="/sys/bus/iio/devices"

mkdir -p "${LOG_DIR}"

###############################################################################
# Counters
###############################################################################

PASS=0
FAIL=0
SKIP=0

SAMPLES=0
INVALID_SAMPLES=0
READ_ERRORS=0

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
# Root Check
###############################################################################

if [ "${EUID}" -ne 0 ]; then

    fail "Run ADC stress test as root."

    echo
    echo "Usage:"
    echo "  sudo ./adc_stress.sh"
    echo "  sudo ./adc_stress.sh 60 10"

    exit 1
fi

###############################################################################
# Parameter Validation
###############################################################################

if ! [[ "${DURATION}" =~ ^[0-9]+$ ]] || [ "${DURATION}" -le 0 ]; then

    fail "Duration must be a positive integer."

    exit 1
fi

if ! [[ "${INTERVAL_MS}" =~ ^[0-9]+$ ]] || [ "${INTERVAL_MS}" -le 0 ]; then

    fail "Interval must be a positive integer."

    exit 1
fi

###############################################################################
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - ADC Stress Test"
log "============================================================"
log "Date          : $(date)"
log "Kernel        : $(uname -r)"
log "Duration      : ${DURATION} seconds"
log "Sample period : ${INTERVAL_MS} ms"
log "IIO path      : ${IIO_ROOT}"
log "Log file      : ${LOG_FILE}"
log "============================================================"
log ""

###############################################################################
# Test 1 - IIO Subsystem
###############################################################################

log "------------------------------------------------------------"
log "TEST 1: IIO subsystem"
log "------------------------------------------------------------"

if [ -d "${IIO_ROOT}" ]; then

    pass "IIO subsystem is available."

else

    fail "IIO subsystem is not available."

    exit 1
fi

###############################################################################
# Find ADC Devices
###############################################################################

ADC_DEVICES=()

for device in "${IIO_ROOT}"/iio:device*; do

    [ -d "${device}" ] || continue

    if [ -f "${device}/name" ]; then

        ADC_NAME="$(tr -d '\0\n' < "${device}/name")"

        ADC_DEVICES+=("${device}")

        log "Detected IIO device: $(basename "${device}")"
        log "Device name       : ${ADC_NAME}"

    fi

done

if [ "${#ADC_DEVICES[@]}" -eq 0 ]; then

    fail "No IIO/ADC devices detected."

    exit 1
fi

###############################################################################
# Discover ADC Channels
###############################################################################

ADC_CHANNELS=()

for device in "${ADC_DEVICES[@]}"; do

    while IFS= read -r channel; do

        ADC_CHANNELS+=("${channel}")

    done < <(
        find "${device}" \
            -maxdepth 1 \
            -type f \
            -name 'in_voltage*_raw' \
            2>/dev/null
    )

done

###############################################################################
# Channel Check
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 2: ADC channel discovery"
log "------------------------------------------------------------"

if [ "${#ADC_CHANNELS[@]}" -eq 0 ]; then

    fail "No ADC voltage channels found."

    exit 1
fi

log "ADC channels detected: ${#ADC_CHANNELS[@]}"

for channel in "${ADC_CHANNELS[@]}"; do

    log "Channel: ${channel}"

done

pass "ADC channels discovered successfully."

###############################################################################
# Display ADC Configuration
###############################################################################

log ""
log "------------------------------------------------------------"
log "ADC Configuration"
log "------------------------------------------------------------"

for device in "${ADC_DEVICES[@]}"; do

    DEVICE_NAME="$(tr -d '\0\n' < "${device}/name" 2>/dev/null || echo "unknown")"

    log "Device: ${device}"
    log "Name  : ${DEVICE_NAME}"

    if [ -f "${device}/sampling_frequency" ]; then

        log "Sampling frequency: $(cat "${device}/sampling_frequency")"

    fi

    if [ -f "${device}/in_voltage_scale ]; then

        log "Voltage scale: $(cat "${device}/in_voltage_scale")"

    fi

done

###############################################################################
# Initial ADC Read Test
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 3: Initial ADC read"
log "------------------------------------------------------------"

INITIAL_FAILURE=0

for channel in "${ADC_CHANNELS[@]}"; do

    VALUE="$(cat "${channel}" 2>/dev/null || true)"

    if [[ "${VALUE}" =~ ^-?[0-9]+$ ]]; then

        log "$(basename "${channel}") = ${VALUE}"

    else

        log "[ERROR] Invalid ADC reading from ${channel}"

        INITIAL_FAILURE=$((INITIAL_FAILURE + 1))

    fi

done

if [ "${INITIAL_FAILURE}" -eq 0 ]; then

    pass "Initial ADC readings are valid."

else

    fail "Initial ADC read test failed."

fi

###############################################################################
# Stress Test Preparation
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 4: ADC continuous sampling"
log "------------------------------------------------------------"

log "Starting ADC stress test..."
log "Duration: ${DURATION} seconds"

START_TIME="$(date +%s)"

NEXT_REPORT="${START_TIME}"

###############################################################################
# Continuous Sampling
###############################################################################

while true; do

    CURRENT_TIME="$(date +%s)"

    ELAPSED=$((CURRENT_TIME - START_TIME))

    if [ "${ELAPSED}" -ge "${DURATION}" ]; then
        break
    fi

    for channel in "${ADC_CHANNELS[@]}"; do

        VALUE="$(cat "${channel}" 2>/dev/null || true)"

        if [[ "${VALUE}" =~ ^-?[0-9]+$ ]]; then

            SAMPLES=$((SAMPLES + 1))

        else

            INVALID_SAMPLES=$((INVALID_SAMPLES + 1))
            READ_ERRORS=$((READ_ERRORS + 1))

        fi

    done

    CURRENT_TIME="$(date +%s)"

    if [ "${CURRENT_TIME}" -ge "${NEXT_REPORT}" ]; then

        log "[INFO] Elapsed: ${ELAPSED}s | Samples: ${SAMPLES} | Errors: ${READ_ERRORS}"

        NEXT_REPORT=$((CURRENT_TIME + 5))

    fi

    sleep "$(awk "BEGIN { printf \"%.3f\", ${INTERVAL_MS}/1000 }")"

done

###############################################################################
# Stress Test Result
###############################################################################

END_TIME="$(date +%s)"

ACTUAL_DURATION=$((END_TIME - START_TIME))

if [ "${ACTUAL_DURATION}" -le 0 ]; then
    ACTUAL_DURATION=1
fi

###############################################################################
# Calculate Sampling Rate
###############################################################################

SAMPLE_RATE="$(
    awk "BEGIN {
        printf \"%.2f\", ${SAMPLES} / ${ACTUAL_DURATION}
    }"
)"

###############################################################################
# Error Rate
###############################################################################

if [ "${SAMPLES}" -gt 0 ]; then

    ERROR_RATE="$(
        awk "BEGIN {
            printf \"%.4f\", (${INVALID_SAMPLES} / (${SAMPLES} + ${INVALID_SAMPLES})) * 100
        }"
    )"

else

    ERROR_RATE="100.0000"

fi

###############################################################################
# Results
###############################################################################

log ""
log "------------------------------------------------------------"
log "ADC STRESS TEST RESULTS"
log "------------------------------------------------------------"

log "Requested duration : ${DURATION} sec"
log "Actual duration    : ${ACTUAL_DURATION} sec"
log "Sample interval    : ${INTERVAL_MS} ms"
log "Valid samples      : ${SAMPLES}"
log "Invalid samples    : ${INVALID_SAMPLES}"
log "Read errors        : ${READ_ERRORS}"
log "Average sample rate: ${SAMPLE_RATE} samples/sec"
log "Error rate         : ${ERROR_RATE}%"

###############################################################################
# ADC Range Check
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 5: ADC reading range"
log "------------------------------------------------------------"

RANGE_ERRORS=0

for channel in "${ADC_CHANNELS[@]}"; do

    VALUE="$(cat "${channel}" 2>/dev/null || true)"

    if [[ "${VALUE}" =~ ^-?[0-9]+$ ]]; then

        if [ "${VALUE}" -lt 0 ]; then

            log "[WARNING] Negative ADC value: ${channel} = ${VALUE}"

        fi

    else

        RANGE_ERRORS=$((RANGE_ERRORS + 1))

    fi

done

if [ "${RANGE_ERRORS}" -eq 0 ]; then

    pass "ADC readings remain within valid numeric range."

else

    fail "ADC range validation detected errors."

fi

###############################################################################
# Kernel ADC Error Scan
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 6: ADC kernel error scan"
log "------------------------------------------------------------"

ADC_ERRORS="$(
    dmesg 2>/dev/null |
    grep -iE "adc|iio|ain|ti_am335x|error|fail" |
    tail -50
)"

if [ -n "${ADC_ERRORS}" ]; then

    log "Recent ADC/IIO-related kernel messages:"
    echo "${ADC_ERRORS}" | tee -a "${LOG_FILE}"

else

    log "No ADC/IIO error messages found."

fi

pass "ADC kernel log scan completed."

###############################################################################
# Final Evaluation
###############################################################################

log ""
log "------------------------------------------------------------"
log "FINAL ADC STRESS EVALUATION"
log "------------------------------------------------------------"

if [ "${SAMPLES}" -eq 0 ]; then

    fail "No valid ADC samples were collected."

elif [ "${READ_ERRORS}" -gt 0 ]; then

    fail "ADC read errors occurred during stress testing."

else

    pass "ADC stress test completed without read errors."

fi

###############################################################################
# Summary
###############################################################################

log ""
log "============================================================"
log " ADC STRESS TEST SUMMARY"
log "============================================================"

echo "PASS             : ${PASS}" | tee -a "${LOG_FILE}"
echo "FAIL             : ${FAIL}" | tee -a "${LOG_FILE}"
echo "SKIP             : ${SKIP}" | tee -a "${LOG_FILE}"

log ""
log "ADC Devices      : ${#ADC_DEVICES[@]}"
log "ADC Channels     : ${#ADC_CHANNELS[@]}"
log "Valid Samples    : ${SAMPLES}"
log "Invalid Samples  : ${INVALID_SAMPLES}"
log "Read Errors      : ${READ_ERRORS}"
log "Sample Rate      : ${SAMPLE_RATE} samples/sec"
log "Error Rate       : ${ERROR_RATE}%"
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
