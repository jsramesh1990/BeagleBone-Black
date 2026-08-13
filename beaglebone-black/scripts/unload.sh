#!/bin/bash

###############################################################################
# BeagleBone Black - Complete Device Driver Project
#
# unload.sh
#
# Purpose:
#   Unload project-specific kernel modules safely.
#
# Supported drivers:
#   - GPIO
#   - I2C
#   - SPI
#   - UART
#   - PWM
#   - CAN
#   - ADC
#
# Usage:
#   sudo ./scripts/unload.sh
#   sudo ./scripts/unload.sh all
#   sudo ./scripts/unload.sh gpio
#   sudo ./scripts/unload.sh i2c
#   sudo ./scripts/unload.sh spi
#   sudo ./scripts/unload.sh uart
#   sudo ./scripts/unload.sh pwm
#   sudo ./scripts/unload.sh can
#   sudo ./scripts/unload.sh adc
#   sudo ./scripts/unload.sh list
###############################################################################

set -u

###############################################################################
# Colors
###############################################################################

GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[1;33m"
BLUE="\033[0;34m"
NC="\033[0m"

###############################################################################
# Driver Module Names
#
# Change these names if your actual .ko module filenames are different.
###############################################################################

GPIO_MODULE="bbb_gpio"
I2C_MODULE="bbb_i2c"
SPI_MODULE="bbb_spi"
UART_MODULE="bbb_uart"
PWM_MODULE="bbb_pwm"
CAN_MODULE="bbb_can"
ADC_MODULE="bbb_adc"

###############################################################################
# Helper Functions
###############################################################################

info()
{
    echo -e "${GREEN}[INFO]${NC} $1"
}

warning()
{
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

error()
{
    echo -e "${RED}[ERROR]${NC} $1"
}

###############################################################################
# Root Check
###############################################################################

check_root()
{
    if [ "${EUID}" -ne 0 ]; then
        error "This script must be run as root."
        echo
        echo "Use:"
        echo "  sudo ./scripts/unload.sh"
        exit 1
    fi
}

###############################################################################
# Check Module
###############################################################################

module_loaded()
{
    lsmod | awk '{print $1}' | grep -qx "$1"
}

###############################################################################
# Unload Module
###############################################################################

unload_module()
{
    MODULE="$1"

    if module_loaded "${MODULE}"; then

        info "Unloading ${MODULE}..."

        if modprobe -r "${MODULE}"; then
            info "${MODULE} unloaded successfully."
        else
            error "Failed to unload ${MODULE}."
            echo
            echo "The module may be busy or another driver may depend on it."
            echo
            return 1
        fi

    else

        warning "${MODULE} is not currently loaded."

    fi
}

###############################################################################
# GPIO
###############################################################################

unload_gpio()
{
    echo
    echo "------------------------------------------"
    echo " GPIO Driver"
    echo "------------------------------------------"

    unload_module "${GPIO_MODULE}"
}

###############################################################################
# I2C
###############################################################################

unload_i2c()
{
    echo
    echo "------------------------------------------"
    echo " I2C Driver"
    echo "------------------------------------------"

    unload_module "${I2C_MODULE}"
}

###############################################################################
# SPI
###############################################################################

unload_spi()
{
    echo
    echo "------------------------------------------"
    echo " SPI Driver"
    echo "------------------------------------------"

    unload_module "${SPI_MODULE}"
}

###############################################################################
# UART
###############################################################################

unload_uart()
{
    echo
    echo "------------------------------------------"
    echo " UART Driver"
    echo "------------------------------------------"

    unload_module "${UART_MODULE}"
}

###############################################################################
# PWM
###############################################################################

unload_pwm()
{
    echo
    echo "------------------------------------------"
    echo " PWM Driver"
    echo "------------------------------------------"

    unload_module "${PWM_MODULE}"
}

###############################################################################
# CAN
###############################################################################

unload_can()
{
    echo
    echo "------------------------------------------"
    echo " CAN Driver"
    echo "------------------------------------------"

    #
    # Bring down CAN interfaces before removing
    # the custom CAN module.
    #
    if command -v ip >/dev/null 2>&1; then

        CAN_INTERFACES=$(ip -o link show type can 2>/dev/null | \
                         awk -F': ' '{print $2}' | \
                         awk '{print $1}')

        for IFACE in ${CAN_INTERFACES}; do

            info "Bringing down ${IFACE}..."

            ip link set "${IFACE}" down 2>/dev/null || \
                warning "Unable to bring down ${IFACE}"

        done

    fi

    unload_module "${CAN_MODULE}"
}

###############################################################################
# ADC
###############################################################################

unload_adc()
{
    echo
    echo "------------------------------------------"
    echo " ADC Driver"
    echo "------------------------------------------"

    unload_module "${ADC_MODULE}"
}

###############################################################################
# List Modules
###############################################################################

list_modules()
{
    echo
    echo "=========================================="
    echo " BBB Driver Module Status"
    echo "=========================================="

    echo
    printf "%-15s %s\n" "Driver" "Status"
    echo "------------------------------------------"

    for MODULE in \
        "${GPIO_MODULE}" \
        "${I2C_MODULE}" \
        "${SPI_MODULE}" \
        "${UART_MODULE}" \
        "${PWM_MODULE}" \
        "${CAN_MODULE}" \
        "${ADC_MODULE}"
    do

        if module_loaded "${MODULE}"; then
            printf "%-15s ${GREEN}%s${NC}\n" \
                "${MODULE}" \
                "LOADED"
        else
            printf "%-15s ${YELLOW}%s${NC}\n" \
                "${MODULE}" \
                "NOT LOADED"
        fi

    done

    echo
}

###############################################################################
# Unload All
###############################################################################

unload_all()
{
    info "Starting BBB driver unload..."

    #
    # Unload in reverse dependency order.
    #
    # CAN is brought down first because network
    # interfaces may still be using the driver.
    #

    unload_can
    unload_uart
    unload_spi
    unload_i2c
    unload_pwm
    unload_adc
    unload_gpio

    depmod -a

    echo
    info "=========================================="
    info " Driver unload completed"
    info "=========================================="

    list_modules
}

###############################################################################
# Main
###############################################################################

check_root

case "${1:-all}" in

    all)
        unload_all
        ;;

    gpio)
        unload_gpio
        ;;

    i2c)
        unload_i2c
        ;;

    spi)
        unload_spi
        ;;

    uart)
        unload_uart
        ;;

    pwm)
        unload_pwm
        ;;

    can)
        unload_can
        ;;

    adc)
        unload_adc
        ;;

    list)
        list_modules
        ;;

    *)
        echo
        echo "Usage:"
        echo
        echo "  sudo ./scripts/unload.sh all"
        echo "  sudo ./scripts/unload.sh gpio"
        echo "  sudo ./scripts/unload.sh i2c"
        echo "  sudo ./scripts/unload.sh spi"
        echo "  sudo ./scripts/unload.sh uart"
        echo "  sudo ./scripts/unload.sh pwm"
        echo "  sudo ./scripts/unload.sh can"
        echo "  sudo ./scripts/unload.sh adc"
        echo "  sudo ./scripts/unload.sh list"
        echo
        exit 1
        ;;

esac
