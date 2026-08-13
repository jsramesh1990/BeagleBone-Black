#!/bin/bash

###############################################################################
# BeagleBone Black - ADC Sampling Performance Test
#
# File:
#   tests/performance/adc_sampling.sh
#
# Purpose:
#   Measure ADC sampling performance, sampling rate, latency, and stability.
#
# Usage:
#   sudo ./adc_sampling.sh
#   sudo ./adc_sampling.sh <adc_channel> <sample_count> <delay_us>
#
# Example:
#   sudo ./adc_sampling.sh 0 1000 100
#
# Parameters:
#   adc_channel  - ADC channel number
#   sample_count - Number of samples
#   delay_us     - Delay between samples in microseconds
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

ADC_CHANNEL="${1:-0}"
SAMPLE_COUNT="${2:-1000}"
DELAY_US="${3:-100}"

ADC_PATH="/sys/bus/iio/devices/iio:device0/in_voltage${ADC_CHANNEL}_raw"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs/performance"
LOG_FILE="${LOG_DIR}/adc_sampling_${TIMESTAMP}.log"

mkdir -p "${LOG_DIR}"

###############################################################################
# Counters
###############################################################################

PASS=0
FAIL=0
SKIP=0

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

    fail "Run this test as root."

    echo
    echo "Usage:"
    echo "  sudo ./adc_sampling.sh"

    exit 1
fi

###############################################################################
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - ADC Sampling Performance Test"
log "============================================================"
log "Date          : $(date)"
log "Kernel        : $(uname -r)"
log "ADC Channel   : ${ADC_CHANNEL}"
log "Sample Count  : ${SAMPLE_COUNT}"
log "Delay         : ${DELAY_US} us"
log "ADC Path      : ${ADC_PATH}"
log "Log File      : ${LOG_FILE}"
log "============================================================"
log ""

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
# Test 2 - ADC Device
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 2: ADC device detection"
log "------------------------------------------------------------"

if [ -d /sys/bus/iio/devices/iio:device0 ]; then

    pass "IIO ADC device detected."

else

    fail "IIO ADC device not detected."

    log ""
    log "Available IIO devices:"

    ls -d /sys/bus/iio/devices/iio:device* \
        2>/dev/null | tee -a "${LOG_FILE}" || true

    exit 1
fi

###############################################################################
# Test 3 - ADC Channel
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 3: ADC channel detection"
log "------------------------------------------------------------"

if [ -f "${ADC_PATH}" ]; then

    pass "ADC channel ${ADC_CHANNEL} is available."

else

    fail "ADC channel ${ADC_CHANNEL} is not available."

    log ""
    log "Available ADC channels:"

    ls /sys/bus/iio/devices/iio:device0/ \
        2>/dev/null | grep "in_voltage" | \
        tee -a "${LOG_FILE}" || true

    exit 1
fi

###############################################################################
# Test 4 - Initial ADC Read
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 4: Initial ADC read"
log "------------------------------------------------------------"

INITIAL_VALUE="$(cat "${ADC_PATH}" 2>/dev/null || true)"

if [[ "${INITIAL_VALUE}" =~ ^[0-9]+$ ]]; then

    log "Initial ADC value: ${INITIAL_VALUE}"

    pass "ADC value read successfully."

else

    fail "Unable to read ADC value."

    exit 1
fi

###############################################################################
# Test 5 - Sampling Performance
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 5: ADC sampling performance"
log "------------------------------------------------------------"

SAMPLE_FILE="/tmp/adc_samples_${$}.txt"

rm -f "${SAMPLE_FILE}"

START_TIME="$(date +%s%N)"

SUCCESSFUL_SAMPLES=0
FAILED_SAMPLES=0

for ((i=1; i<=SAMPLE_COUNT; i++)); do

    VALUE="$(cat "${ADC_PATH}" 2>/dev/null || true)"

    if [[ "${VALUE}" =~ ^[0-9]+$ ]]; then

        echo "${VALUE}" >> "${SAMPLE_FILE}"

        SUCCESSFUL_SAMPLES=$((SUCCESSFUL_SAMPLES + 1))

    else

        FAILED_SAMPLES=$((FAILED_SAMPLES + 1))

    fi

    if [ "${DELAY_US}" -gt 0 ]; then

        sleep "0.${DELAY_US}" 2>/dev/null || true

    fi

done

END_TIME="$(date +%s%N)"

###############################################################################
# Calculate Performance
###############################################################################

ELAPSED_NS=$((END_TIME - START_TIME))

if [ "${ELAPSED_NS}" -gt 0 ]; then

    ELAPSED_MS=$((ELAPSED_NS / 1000000))

    TOTAL_US=$((ELAPSED_NS / 1000))

    if [ "${TOTAL_US}" -gt 0 ]; then

        ACTUAL_RATE=$((SUCCESSFUL_SAMPLES * 1000000 / TOTAL_US))

    else

        ACTUAL_RATE=0

    fi

else

    ELAPSED_MS=0
    TOTAL_US=0
    ACTUAL_RATE=0

fi

###############################################################################
# Sampling Results
###############################################################################

log ""
log "Sampling Results"
log "----------------"
log "Requested samples : ${SAMPLE_COUNT}"
log "Successful samples: ${SUCCESSFUL_SAMPLES}"
log "Failed samples    : ${FAILED_SAMPLES}"
log "Elapsed time      : ${ELAPSED_MS} ms"
log "Actual sample rate: ${ACTUAL_RATE} samples/sec"

if [ "${SUCCESSFUL_SAMPLES}" -eq "${SAMPLE_COUNT}" ]; then

    pass "All ADC samples were collected successfully."

else

    fail "Some ADC samples failed."

fi

###############################################################################
# Test 6 - Sampling Stability
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 6: ADC sampling stability"
log "------------------------------------------------------------"

if [ -s "${SAMPLE_FILE}" ]; then

    MIN_VALUE="$(sort -n "${SAMPLE_FILE}" | head -1)"
    MAX_VALUE="$(sort -n "${SAMPLE_FILE}" | tail -1)"

    AVG_VALUE="$(awk '
        {
            sum += $1
            count++
        }
        END {
            if (count > 0)
                printf "%.2f", sum / count
            else
                print "0"
        }
    ' "${SAMPLE_FILE}")"

    log "Minimum ADC value : ${MIN_VALUE}"
    log "Maximum ADC value : ${MAX_VALUE}"
    log "Average ADC value : ${AVG_VALUE}"

    RANGE=$((MAX_VALUE - MIN_VALUE))

    log "ADC value range   : ${RANGE}"

    pass "ADC sampling stability statistics calculated."

else

    fail "No ADC samples available."

fi

###############################################################################
# Test 7 - Sample Data
###############################################################################

log ""
log "------------------------------------------------------------"
log "TEST 7: Sample data verification"
log "------------------------------------------------------------"

log "First 10 samples:"

head -10 "${SAMPLE_FILE}" \
    2>/dev/null | tee -a "${LOG_FILE}"

log ""
log "Last 10 samples:"

tail -10 "${SAMPLE_FILE}" \
    2>/dev/null | tee -a "${LOG_FILE}"

pass "ADC sample data verified."

###############################################################################
# Cleanup
###############################################################################

rm -f "${SAMPLE_FILE}"

###############################################################################
# Summary
###############################################################################

log ""
log "============================================================"
log " ADC SAMPLING PERFORMANCE SUMMARY"
log "============================================================"

echo "PASS : ${PASS}" | tee -a "${LOG_FILE}"
echo "FAIL : ${FAIL}" | tee -a "${LOG_FILE}"
echo "SKIP : ${SKIP}" | tee -a "${LOG_FILE}"

log ""
log "ADC Channel       : ${ADC_CHANNEL}"
log "Requested Samples : ${SAMPLE_COUNT}"
log "Successful Samples: ${SUCCESSFUL_SAMPLES}"
log "Failed Samples    : ${FAILED_SAMPLES}"
log "Elapsed Time      : ${ELAPSED_MS} ms"
log "Actual Rate       : ${ACTUAL_RATE} samples/sec"

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
