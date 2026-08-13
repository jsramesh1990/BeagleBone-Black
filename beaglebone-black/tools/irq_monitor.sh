#!/bin/bash

###############################################################################
# BeagleBone Black - IRQ Monitor
#
# File:
#   tools/irq_monitor.sh
#
# Purpose:
#   Monitor Linux interrupt activity for GPIO, I2C, SPI, UART, PWM, CAN,
#   Ethernet, USB, MMC, DMA and other peripheral interrupts.
#
# Usage:
#   sudo ./irq_monitor.sh
#   sudo ./irq_monitor.sh <interval> <samples>
#
# Example:
#   sudo ./irq_monitor.sh 1 60
#
#   interval = sampling interval in seconds
#   samples  = number of samples
#
# Press Ctrl+C to stop when running without a sample limit.
###############################################################################

set -u

###############################################################################
# Configuration
###############################################################################

INTERVAL="${1:-1}"
SAMPLES="${2:-60}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"

LOG_DIR="${PROJECT_ROOT}/logs"
LOG_FILE="${LOG_DIR}/irq_monitor_${TIMESTAMP}.log"

mkdir -p "${LOG_DIR}"

###############################################################################
# Counters
###############################################################################

SAMPLE_COUNT=0

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
    log "============================================================"
    log " IRQ MONITOR STOPPED"
    log "============================================================"
    log "Samples collected : ${SAMPLE_COUNT}"
    log "Log file          : ${LOG_FILE}"
}

trap cleanup EXIT INT TERM

###############################################################################
# Header
###############################################################################

log "============================================================"
log " BeagleBone Black - IRQ Monitor"
log "============================================================"
log "Date        : $(date)"
log "Kernel      : $(uname -r)"
log "Architecture: $(uname -m)"
log "Interval    : ${INTERVAL} seconds"
log "Samples     : ${SAMPLES}"
log "Log File    : ${LOG_FILE}"
log "============================================================"

###############################################################################
# Root Check
###############################################################################

if [ "${EUID}" -ne 0 ]; then

    log "[ERROR] Run IRQ monitor as root."

    exit 1
fi

###############################################################################
# Validate Parameters
###############################################################################

if ! [[ "${INTERVAL}" =~ ^[0-9]+([.][0-9]+)?$ ]]; then

    log "[ERROR] Invalid interval: ${INTERVAL}"

    exit 1
fi

if ! [[ "${SAMPLES}" =~ ^[0-9]+$ ]] || [ "${SAMPLES}" -le 0 ]; then

    log "[ERROR] Samples must be a positive integer."

    exit 1
fi

###############################################################################
# /proc/interrupts Check
###############################################################################

if [ ! -r /proc/interrupts ]; then

    log "[ERROR] /proc/interrupts is not readable."

    exit 1
fi

###############################################################################
# CPU Information
###############################################################################

log ""
log "------------------------------------------------------------"
log "CPU INFORMATION"
log "------------------------------------------------------------"

CPU_COUNT="$(nproc)"

log "CPU cores: ${CPU_COUNT}"

log ""
log "CPU:"
grep -m 1 "model name" /proc/cpuinfo 2>/dev/null |
    tee -a "${LOG_FILE}" || true

###############################################################################
# Initial IRQ Snapshot
###############################################################################

log ""
log "------------------------------------------------------------"
log "INITIAL IRQ SNAPSHOT"
log "------------------------------------------------------------"

cat /proc/interrupts |
    tee -a "${LOG_FILE}"

###############################################################################
# Peripheral IRQ Function
###############################################################################

show_peripheral_irq()
{
    local NAME="$1"

    log ""
    log "### ${NAME} IRQs ###"

    grep -i "${NAME}" /proc/interrupts |
        tee -a "${LOG_FILE}" || true
}

###############################################################################
# Peripheral IRQ Summary
###############################################################################

log ""
log "------------------------------------------------------------"
log "PERIPHERAL IRQ SUMMARY"
log "------------------------------------------------------------"

show_peripheral_irq "gpio"
show_peripheral_irq "i2c"
show_peripheral_irq "spi"
show_peripheral_irq "uart"
show_peripheral_irq "serial"
show_peripheral_irq "pwm"
show_peripheral_irq "can"
show_peripheral_irq "eth"
show_peripheral_irq "usb"
show_peripheral_irq "mmc"
show_peripheral_irq "dma"

###############################################################################
# IRQ Monitor
###############################################################################

log ""
log "============================================================"
log " CONTINUOUS IRQ MONITOR"
log "============================================================"
log ""

log "Format:"
log "  IRQ number | CPU interrupt count | Interrupt source"
log ""

###############################################################################
# Temporary Files
###############################################################################

PREVIOUS_FILE="$(mktemp)"
CURRENT_FILE="$(mktemp)"

cleanup_temp()
{
    rm -f "${PREVIOUS_FILE}" "${CURRENT_FILE}"
}

trap cleanup_temp EXIT INT TERM

###############################################################################
# Save Initial Snapshot
###############################################################################

awk '
    /^[[:space:]]*[0-9]+:/ {
        irq=$1
        sub(":", "", irq)

        total=0

        for (i=2; i<=NF; i++) {
            if ($i ~ /^[0-9]+$/)
                total += $i
        }

        source=""

        for (i=2; i<=NF; i++) {
            if ($i !~ /^[0-9]+$/ && $i != "-") {
                source=source " " $i
            }
        }

        print irq "|" total "|" source
    }
' /proc/interrupts > "${PREVIOUS_FILE}"

###############################################################################
# Monitor Loop
###############################################################################

while [ "${SAMPLE_COUNT}" -lt "${SAMPLES}" ]; do

    SAMPLE_COUNT=$((SAMPLE_COUNT + 1))

    sleep "${INTERVAL}"

    TIMESTAMP_NOW="$(date '+%Y-%m-%d %H:%M:%S')"

    ###########################################################################
    # Capture Current IRQ State
    ###########################################################################

    awk '
        /^[[:space:]]*[0-9]+:/ {
            irq=$1
            sub(":", "", irq)

            total=0

            for (i=2; i<=NF; i++) {
                if ($i ~ /^[0-9]+$/)
                    total += $i
            }

            source=""

            for (i=2; i<=NF; i++) {
                if ($i !~ /^[0-9]+$/ && $i != "-") {
                    source=source " " $i
                }
            }

            print irq "|" total "|" source
        }
    ' /proc/interrupts > "${CURRENT_FILE}"

    ###########################################################################
    # Display IRQ Rate
    ###########################################################################

    log ""
    log "------------------------------------------------------------"
    log "SAMPLE ${SAMPLE_COUNT}/${SAMPLES} - ${TIMESTAMP_NOW}"
    log "------------------------------------------------------------"

    awk -F'|' \
        -v interval="${INTERVAL}" \
        '
        BEGIN {
            while ((getline line < "'"${PREVIOUS_FILE}"'") > 0) {

                split(line, a, "|")

                previous[a[1]] = a[2]
                source[a[1]] = a[3]
            }

            close("'"${PREVIOUS_FILE}"'")
        }

        {
            irq=$1
            current=$2
            name=$3

            old=previous[irq]

            if (old == "")
                old=0

            delta=current-old

            if (delta > 0) {

                rate=delta/interval

                printf "%-6s %-12s %-12s %s\n",
                    irq,
                    delta,
                    sprintf("%.2f/s", rate),
                    name
            }
        }
        ' "${CURRENT_FILE}" |
        sort -k2 -nr |
        head -50 |
        tee -a "${LOG_FILE}"

    ###########################################################################
    # Peripheral IRQ Changes
    ###########################################################################

    log ""
    log "Peripheral IRQ activity:"

    awk -F'|' \
        -v interval="${INTERVAL}" \
        '
        BEGIN {

            while ((getline line < "'"${PREVIOUS_FILE}"'") > 0) {

                split(line, a, "|")

                previous[a[1]] = a[2]
                source[a[1]] = a[3]
            }

            close("'"${PREVIOUS_FILE}"'")
        }

        {
            irq=$1
            current=$2
            name=$3

            old=previous[irq]

            if (old == "")
                old=0

            delta=current-old

            if (delta > 0 &&
                name ~ /gpio|i2c|spi|uart|serial|pwm|can|eth|usb|mmc|dma/i) {

                printf "%-6s %-12s %-12s %s\n",
                    irq,
                    delta,
                    sprintf("%.2f/s", delta/interval),
                    name
            }
        }
        ' "${CURRENT_FILE}" |
        tee -a "${LOG_FILE}"

    ###########################################################################
    # Save Snapshot
    ###########################################################################

    cp "${CURRENT_FILE}" "${PREVIOUS_FILE}"

done

###############################################################################
# Final IRQ Snapshot
###############################################################################

log ""
log "============================================================"
log "FINAL IRQ SNAPSHOT"
log "============================================================"

cat /proc/interrupts |
    tee -a "${LOG_FILE}"

###############################################################################
# IRQ Affinity Information
###############################################################################

log ""
log "------------------------------------------------------------"
log "IRQ AFFINITY"
log "------------------------------------------------------------"

if [ -d /proc/irq ]; then

    for IRQ_DIR in /proc/irq/[0-9]*; do

        [ -d "${IRQ_DIR}" ] || continue

        IRQ="$(basename "${IRQ_DIR}")"

        if [ -f "${IRQ_DIR}/smp_affinity_list" ]; then

            AFFINITY="$(
                cat "${IRQ_DIR}/smp_affinity_list" 2>/dev/null
            )"

            if [ -n "${AFFINITY}" ]; then

                log "IRQ ${IRQ}: CPU ${AFFINITY}"

            fi

        fi

    done

fi

###############################################################################
# IRQ Statistics
###############################################################################

log ""
log "------------------------------------------------------------"
log "IRQ STATISTICS"
log "------------------------------------------------------------"

if [ -f /proc/softirqs ]; then

    log "SoftIRQ statistics:"
    cat /proc/softirqs |
        tee -a "${LOG_FILE}"
fi

###############################################################################
# Kernel IRQ Messages
###############################################################################

log ""
log "------------------------------------------------------------"
log "IRQ-RELATED KERNEL MESSAGES"
log "------------------------------------------------------------"

IRQ_MESSAGES="$(
    dmesg 2>/dev/null |
    grep -iE \
    "irq|interrupt|gpio.*irq|i2c.*irq|spi.*irq|uart.*irq|"
    "can.*irq|usb.*irq|dma.*irq|irq.*error|irq.*failed" |
    tail -100
)"

if [ -n "${IRQ_MESSAGES}" ]; then

    echo "${IRQ_MESSAGES}" |
        tee -a "${LOG_FILE}"

else

    log "No IRQ-related kernel messages found."

fi

###############################################################################
# Final Summary
###############################################################################

log ""
log "============================================================"
log " IRQ MONITOR SUMMARY"
log "============================================================"

log "Samples collected : ${SAMPLE_COUNT}"
log "Sampling interval : ${INTERVAL} seconds"

log ""
log "IRQ monitoring completed successfully."

log ""
log "Results saved to:"
log "${LOG_FILE}"

log ""
log "============================================================"
log " END OF IRQ MONITOR"
log "============================================================"

exit 0
