#!/bin/bash

###############################################################################
# BeagleBone Black - Complete Device Driver Project
#
# Test All Peripherals
#
# Tests:
#   - GPIO
#   - ADC
#   - I2C
#   - SPI
#   - UART
#   - PWM
#   - CAN
#   - Device Tree
#   - Kernel Modules
#
# Usage:
#   sudo ./scripts/test_all.sh
#   sudo ./scripts/test_all.sh gpio
#   sudo ./scripts/test_all.sh adc
#   sudo ./scripts/test_all.sh i2c
#   sudo ./scripts/test_all.sh spi
#   sudo ./scripts/test_all.sh uart
#   sudo ./scripts/test_all.sh pwm
#   sudo ./scripts/test_all.sh can
#   sudo ./scripts/test_all.sh dt
#   sudo ./scripts/test_all.sh modules
###############################################################################

set -u

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

LOG_DIR="${PROJECT_ROOT}/logs"
TIMESTAMP="$(date '+%Y%m%d_%H%M%S')"
TEST_LOG="${LOG_DIR}/test_${TIMESTAMP}.log"

mkdir -p "${LOG_DIR}"

PASS=0
FAIL=0
SKIP=0

###############################################################################
# Colors
###############################################################################

GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[1;33m"
BLUE="\033[0;34m"
NC="\033[0m"

###############################################################################
# Logging
###############################################################################

log()
{
    echo "$1" | tee -a "${TEST_LOG}"
}

pass()
{
    echo -e "${GREEN}[PASS]${NC} $1" | tee -a "${TEST_LOG}"
    PASS=$((PASS + 1))
}

fail()
{
    echo -e "${RED}[FAIL]${NC} $1" | tee -a "${TEST_LOG}"
    FAIL=$((FAIL + 1))
}

skip()
{
    echo -e "${YELLOW}[SKIP]${NC} $1" | tee -a "${TEST_LOG}"
    SKIP=$((SKIP + 1))
}

###############################################################################
# Command Check
###############################################################################

command_exists()
{
    command -v "$1" >/dev/null 2>&1
}

###############################################################################
# GPIO Test
###############################################################################

test_gpio()
{
    log ""
    log "=========================================="
    log " GPIO TEST"
    log "=========================================="

    if [ -d /sys/class/gpio ]; then
        pass "GPIO subsystem available"
    else
        fail "GPIO subsystem not available"
        return
    fi

    if [ -d /sys/class/gpiochip ]; then
        GPIO_CHIPS=$(find /sys/class/gpiochip -maxdepth 1 -type l 2>/dev/null | wc -l)

        if [ "${GPIO_CHIPS}" -gt 0 ]; then
            pass "GPIO chips detected: ${GPIO_CHIPS}"
        else
            fail "No GPIO chips detected"
        fi
    else
        skip "gpiochip sysfs interface unavailable"
    fi

    if command_exists gpiodetect; then
        gpiodetect | tee -a "${TEST_LOG}"
        pass "GPIO character-device interface tested"
    else
        skip "gpiodetect not installed"
    fi
}

###############################################################################
# ADC / IIO Test
###############################################################################

test_adc()
{
    log ""
    log "=========================================="
    log " ADC / IIO TEST"
    log "=========================================="

    if [ ! -d /sys/bus/iio/devices ]; then
        fail "IIO subsystem not available"
        return
    fi

    DEVICES=$(find /sys/bus/iio/devices \
        -maxdepth 1 \
        -type l 2>/dev/null | wc -l)

    if [ "${DEVICES}" -gt 0 ]; then
        pass "IIO devices detected: ${DEVICES}"
    else
        fail "No IIO devices detected"
        return
    fi

    for DEV in /sys/bus/iio/devices/iio:device*; do

        [ -d "${DEV}" ] || continue

        log "Checking ${DEV}"

        CHANNELS=$(find "${DEV}" \
            -maxdepth 1 \
            -name "in_voltage*_raw" \
            2>/dev/null)

        if [ -n "${CHANNELS}" ]; then

            while read -r CHANNEL; do

                VALUE=$(cat "${CHANNEL}" 2>/dev/null || true)

                if [ -n "${VALUE}" ]; then
                    pass "ADC channel $(basename "${CHANNEL}") = ${VALUE}"
                else
                    fail "Unable to read ${CHANNEL}"
                fi

            done <<< "${CHANNELS}"

        else
            skip "No voltage ADC channels found in ${DEV}"
        fi
    done
}

###############################################################################
# I2C Test
###############################################################################

test_i2c()
{
    log ""
    log "=========================================="
    log " I2C TEST"
    log "=========================================="

    if [ ! -d /sys/class/i2c-adapter ]; then
        fail "I2C subsystem not available"
        return
    fi

    I2C_COUNT=$(find /sys/class/i2c-adapter \
        -maxdepth 1 \
        -type l 2>/dev/null | wc -l)

    if [ "${I2C_COUNT}" -gt 0 ]; then
        pass "I2C adapters detected: ${I2C_COUNT}"
    else
        fail "No I2C adapters detected"
        return
    fi

    if command_exists i2cdetect; then

        i2cdetect -l | tee -a "${TEST_LOG}"

        while read -r BUS; do

            [ -n "${BUS}" ] || continue

            BUS_NUM=$(echo "${BUS}" | sed -n 's/^i2c-\([0-9]*\).*/\1/p')

            if [ -n "${BUS_NUM}" ]; then
                log "Scanning I2C bus ${BUS_NUM}"

                i2cdetect -y "${BUS_NUM}" 2>&1 | tee -a "${TEST_LOG}"
            fi

        done < <(i2cdetect -l 2>/dev/null)

        pass "I2C bus scan completed"

    else
        skip "i2cdetect not installed"
    fi
}

###############################################################################
# SPI Test
###############################################################################

test_spi()
{
    log ""
    log "=========================================="
    log " SPI TEST"
    log "=========================================="

    if [ ! -d /sys/bus/spi/devices ]; then
        fail "SPI subsystem not available"
        return
    fi

    SPI_COUNT=$(find /sys/bus/spi/devices \
        -maxdepth 1 \
        -type l 2>/dev/null | wc -l)

    if [ "${SPI_COUNT}" -gt 0 ]; then
        pass "SPI devices detected: ${SPI_COUNT}"
    else
        skip "No SPI devices currently registered"
    fi

    if [ -e /dev/spidev0.0 ]; then
        pass "/dev/spidev0.0 available"
    else
        skip "No spidev0.0 device"
    fi

    if command_exists spidev_test; then

        if [ -e /dev/spidev0.0 ]; then
            log "Running SPI loopback test"

            if spidev_test -D /dev/spidev0.0 2>&1 | \
                tee -a "${TEST_LOG}"; then
                pass "SPI test completed"
            else
                fail "SPI test failed"
            fi
        else
            skip "SPI test skipped - no spidev device"
        fi

    else
        skip "spidev_test not installed"
    fi
}

###############################################################################
# UART Test
###############################################################################

test_uart()
{
    log ""
    log "=========================================="
    log " UART TEST"
    log "=========================================="

    UART_COUNT=$(find /dev \
        -maxdepth 1 \
        -name "ttyS*" \
        -o -name "ttyO*" \
        2>/dev/null | wc -l)

    if [ "${UART_COUNT}" -gt 0 ]; then
        pass "UART devices detected: ${UART_COUNT}"
    else
        fail "No UART devices detected"
    fi

    log "Available serial devices:"

    ls -l /dev/ttyS* /dev/ttyO* /dev/ttyUSB* /dev/ttyACM* \
        2>/dev/null | tee -a "${TEST_LOG}" || true

    if command_exists stty; then

        for UART in /dev/ttyS* /dev/ttyO*; do

            [ -e "${UART}" ] || continue

            log "UART configuration: ${UART}"

            stty -F "${UART}" -a 2>&1 | \
                tee -a "${TEST_LOG}" || true

        done

        pass "UART device inspection completed"

    else
        skip "stty not available"
    fi
}

###############################################################################
# PWM Test
###############################################################################

test_pwm()
{
    log ""
    log "=========================================="
    log " PWM TEST"
    log "=========================================="

    if [ ! -d /sys/class/pwm ]; then
        fail "PWM subsystem not available"
        return
    fi

    PWMCHIPS=$(find /sys/class/pwm \
        -maxdepth 1 \
        -type l \
        2>/dev/null | wc -l)

    if [ "${PWMCHIPS}" -gt 0 ]; then
        pass "PWM chips detected: ${PWMCHIPS}"
    else
        skip "No PWM chips detected"
        return
    fi

    for CHIP in /sys/class/pwm/pwmchip*; do

        [ -d "${CHIP}" ] || continue

        log "PWM chip: ${CHIP}"

        cat "${CHIP}/npwm" 2>/dev/null | \
            tee -a "${TEST_LOG}" || true

        find "${CHIP}" \
            -maxdepth 1 \
            -name "pwm*" \
            -print 2>/dev/null | \
            tee -a "${TEST_LOG}"

    done

    pass "PWM subsystem inspection completed"
}

###############################################################################
# CAN Test
###############################################################################

test_can()
{
    log ""
    log "=========================================="
    log " CAN TEST"
    log "=========================================="

    if ! command_exists ip; then
        fail "ip command not available"
        return
    fi

    CAN_IFACES=$(ip -o link show type can 2>/dev/null || true)

    if [ -n "${CAN_IFACES}" ]; then

        echo "${CAN_IFACES}" | tee -a "${TEST_LOG}"

        pass "CAN interface detected"

        ip -details link show type can | \
            tee -a "${TEST_LOG}"

        if command_exists candump; then

            CAN_DEV=$(echo "${CAN_IFACES}" | \
                awk -F': ' '{print $2}' | \
                awk '{print $1}' | head -n1)

            if [ -n "${CAN_DEV}" ]; then
                log "CAN interface: ${CAN_DEV}"
                ip -details link show "${CAN_DEV}" | \
                    tee -a "${TEST_LOG}"
            fi

        else
            skip "candump not installed"
        fi

    else
        skip "No CAN interface detected"
    fi
}

###############################################################################
# Device Tree Test
###############################################################################

test_device_tree()
{
    log ""
    log "=========================================="
    log " DEVICE TREE TEST"
    log "=========================================="

    if [ ! -d /proc/device-tree ]; then
        fail "/proc/device-tree not available"
        return
    fi

    pass "Device Tree filesystem available"

    if [ -f /proc/device-tree/model ]; then

        MODEL=$(tr '\0' '\n' < /proc/device-tree/model)

        log "Board: ${MODEL}"

        pass "Board Device Tree identified"

    else
        fail "Device Tree model not found"
    fi

    if [ -f /proc/device-tree/compatible ]; then

        log "Compatible strings:"

        tr '\0' '\n' < /proc/device-tree/compatible | \
            tee -a "${TEST_LOG}"

        pass "Device Tree compatible information available"

    fi
}

###############################################################################
# Kernel Module Test
###############################################################################

test_modules()
{
    log ""
    log "=========================================="
    log " KERNEL MODULE TEST"
    log "=========================================="

    if [ ! -f /proc/modules ]; then
        fail "/proc/modules unavailable"
        return
    fi

    MODULE_COUNT=$(wc -l < /proc/modules)

    log "Loaded modules: ${MODULE_COUNT}"

    if [ "${MODULE_COUNT}" -gt 0 ]; then
        pass "Kernel modules detected"
    else
        skip "No loadable modules currently loaded"
    fi

    log "BBB project modules:"

    lsmod | grep -E \
        'bbb|gpio|i2c|spi|serial|pwm|can|adc|iio' \
        2>/dev/null | tee -a "${TEST_LOG}" || \
        skip "No matching BBB/peripheral modules loaded"
}

###############################################################################
# Interrupt Test
###############################################################################

test_interrupts()
{
    log ""
    log "=========================================="
    log " INTERRUPT TEST"
    log "=========================================="

    if [ -f /proc/interrupts ]; then

        cat /proc/interrupts | \
            tee -a "${TEST_LOG}"

        pass "Interrupt table available"

    else

        fail "/proc/interrupts unavailable"

    fi
}

###############################################################################
# Run All Tests
###############################################################################

test_all()
{
    log ""
    log "############################################################"
    log "# BeagleBone Black - Complete Peripheral Test"
    log "############################################################"
    log "# Date: $(date)"
    log "# Kernel: $(uname -r)"
    log "# Log: ${TEST_LOG}"
    log "############################################################"

    test_device_tree
    test_modules
    test_gpio
    test_adc
    test_i2c
    test_spi
    test_uart
    test_pwm
    test_can
    test_interrupts

    print_summary
}

###############################################################################
# Summary
###############################################################################

print_summary()
{
    log ""
    log "############################################################"
    log "# TEST SUMMARY"
    log "############################################################"

    echo -e "${GREEN}PASS : ${PASS}${NC}" | tee -a "${TEST_LOG}"
    echo -e "${RED}FAIL : ${FAIL}${NC}" | tee -a "${TEST_LOG}"
    echo -e "${YELLOW}SKIP : ${SKIP}${NC}" | tee -a "${TEST_LOG}"

    log ""
    log "Test log:"
    log "${TEST_LOG}"

    if [ "${FAIL}" -eq 0 ]; then
        log ""
        log "Overall result: PASS"
        return 0
    else
        log ""
        log "Overall result: FAIL"
        return 1
    fi
}

###############################################################################
# Main
###############################################################################

case "${1:-all}" in

    all)
        test_all
        ;;

    gpio)
        test_gpio
        print_summary
        ;;

    adc)
        test_adc
        print_summary
        ;;

    i2c)
        test_i2c
        print_summary
        ;;

    spi)
        test_spi
        print_summary
        ;;

    uart)
        test_uart
        print_summary
        ;;

    pwm)
        test_pwm
        print_summary
        ;;

    can)
        test_can
        print_summary
        ;;

    dt)
        test_device_tree
        print_summary
        ;;

    modules)
        test_modules
        print_summary
        ;;

    interrupts)
        test_interrupts
        print_summary
        ;;

    *)
        echo "Usage:"
        echo
        echo "  sudo ./scripts/test_all.sh all"
        echo "  sudo ./scripts/test_all.sh gpio"
        echo "  sudo ./scripts/test_all.sh adc"
        echo "  sudo ./scripts/test_all.sh i2c"
        echo "  sudo ./scripts/test_all.sh spi"
        echo "  sudo ./scripts/test_all.sh uart"
        echo "  sudo ./scripts/test_all.sh pwm"
        echo "  sudo ./scripts/test_all.sh can"
        echo "  sudo ./scripts/test_all.sh dt"
        echo "  sudo ./scripts/test_all.sh modules"
        echo "  sudo ./scripts/test_all.sh interrupts"
        exit 1
        ;;

esac
