#!/bin/bash

###############################################################################
# BeagleBone Black - Complete Device Driver Project
#
# Installation Script
#
# Installs:
#   - Kernel Image
#   - Device Tree Blobs
#   - Kernel Modules
#   - Device Tree Overlays
#   - Custom Driver Modules
#
# Usage:
#   sudo ./scripts/install.sh
#   sudo ./scripts/install.sh all
#   sudo ./scripts/install.sh kernel
#   sudo ./scripts/install.sh dtb
#   sudo ./scripts/install.sh modules
#   sudo ./scripts/install.sh drivers
#   sudo ./scripts/install.sh clean
###############################################################################

set -e

###############################################################################
# Paths
###############################################################################

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

OUTPUT_DIR="${PROJECT_ROOT}/output"

BOOT_DIR="/boot"
MODULE_DIR="/lib/modules"

KERNEL_IMAGE="${OUTPUT_DIR}/zImage"
DTB_DIR="${OUTPUT_DIR}/dtbs"
MODULES_DIR="${OUTPUT_DIR}/modules"

###############################################################################
# Colors
###############################################################################

GREEN="\033[0;32m"
YELLOW="\033[1;33m"
RED="\033[0;31m"
NC="\033[0m"

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
    if [ "$EUID" -ne 0 ]; then
        error "This script must be run as root."
        echo
        echo "Use:"
        echo "  sudo ./scripts/install.sh"
        exit 1
    fi
}

###############################################################################
# Output Check
###############################################################################

check_output()
{
    if [ ! -d "${OUTPUT_DIR}" ]; then
        error "Output directory not found:"
        echo "${OUTPUT_DIR}"
        echo
        echo "Run the build first:"
        echo "  ./scripts/build.sh all"
        exit 1
    fi
}

###############################################################################
# Install Kernel
###############################################################################

install_kernel()
{
    info "Installing Linux kernel..."

    if [ ! -f "${KERNEL_IMAGE}" ]; then
        error "Kernel image not found:"
        echo "${KERNEL_IMAGE}"
        exit 1
    fi

    cp "${KERNEL_IMAGE}" \
       "${BOOT_DIR}/zImage-bbb"

    sync

    info "Kernel installed:"
    echo "${BOOT_DIR}/zImage-bbb"
}

###############################################################################
# Install Device Tree
###############################################################################

install_dtb()
{
    info "Installing Device Tree files..."

    if [ ! -d "${DTB_DIR}" ]; then
        error "DTB directory not found:"
        echo "${DTB_DIR}"
        exit 1
    fi

    mkdir -p "${BOOT_DIR}/dtbs"

    find "${DTB_DIR}" \
        -type f \
        \( -name "*.dtb" -o -name "*.dtbo" \) \
        -exec cp {} "${BOOT_DIR}/dtbs/" \;

    sync

    info "Device Tree files installed."
}

###############################################################################
# Install Kernel Modules
###############################################################################

install_modules()
{
    info "Installing kernel modules..."

    if [ ! -d "${MODULES_DIR}" ]; then
        error "Kernel modules directory not found:"
        echo "${MODULES_DIR}"
        exit 1
    fi

    cp -a "${MODULES_DIR}/." "/"

    depmod -a

    sync

    info "Kernel modules installed."
}

###############################################################################
# Install Custom Drivers
###############################################################################

install_drivers()
{
    info "Installing project driver modules..."

    DRIVER_INSTALL_DIR="/lib/modules/$(uname -r)/extra/bbb"

    mkdir -p "${DRIVER_INSTALL_DIR}"

    if [ -d "${PROJECT_ROOT}/drivers" ]; then

        find "${PROJECT_ROOT}/drivers" \
            -type f \
            -name "*.ko" \
            -exec cp {} "${DRIVER_INSTALL_DIR}/" \;

    else

        warning "drivers directory not found."

    fi

    depmod -a

    info "Custom driver installation completed."

    echo
    echo "Driver directory:"
    echo "${DRIVER_INSTALL_DIR}"
}

###############################################################################
# Install Everything
###############################################################################

install_all()
{
    check_root
    check_output

    info "Starting complete BeagleBone Black installation..."
    echo

    install_kernel
    install_dtb
    install_modules
    install_drivers

    echo
    info "=========================================="
    info "Installation completed successfully"
    info "=========================================="
    echo

    echo "Installed components:"
    echo
    echo "Kernel:"
    echo "  ${BOOT_DIR}/zImage-bbb"
    echo
    echo "Device Tree:"
    echo "  ${BOOT_DIR}/dtbs/"
    echo
    echo "Modules:"
    echo "  ${MODULE_DIR}/"
    echo
    echo "Drivers:"
    echo "  /lib/modules/$(uname -r)/extra/bbb/"
    echo

    sync
}

###############################################################################
# Clean Installed Files
###############################################################################

clean_install()
{
    check_root

    warning "Removing project-installed kernel and Device Tree files..."

    rm -f "${BOOT_DIR}/zImage-bbb"

    if [ -d "${BOOT_DIR}/dtbs" ]; then
        find "${BOOT_DIR}/dtbs" \
            -type f \
            \( -name "*.dtb" -o -name "*.dtbo" \) \
            -delete
    fi

    DRIVER_INSTALL_DIR="/lib/modules/$(uname -r)/extra/bbb"

    if [ -d "${DRIVER_INSTALL_DIR}" ]; then
        rm -rf "${DRIVER_INSTALL_DIR}"
    fi

    depmod -a

    sync

    info "Project installation files removed."
}

###############################################################################
# Show Help
###############################################################################

show_help()
{
    echo
    echo "BeagleBone Black Device Driver Installation"
    echo
    echo "Usage:"
    echo
    echo "  sudo ./scripts/install.sh"
    echo "      Install everything"
    echo
    echo "  sudo ./scripts/install.sh all"
    echo "      Install kernel, DTB, modules and drivers"
    echo
    echo "  sudo ./scripts/install.sh kernel"
    echo "      Install Linux kernel"
    echo
    echo "  sudo ./scripts/install.sh dtb"
    echo "      Install Device Tree files"
    echo
    echo "  sudo ./scripts/install.sh modules"
    echo "      Install kernel modules"
    echo
    echo "  sudo ./scripts/install.sh drivers"
    echo "      Install project driver modules"
    echo
    echo "  sudo ./scripts/install.sh clean"
    echo "      Remove project-installed files"
    echo
    echo "  sudo ./scripts/install.sh help"
    echo "      Show this help"
    echo
}

###############################################################################
# Main
###############################################################################

case "$1" in

    all|"")
        install_all
        ;;

    kernel)
        check_root
        check_output
        install_kernel
        ;;

    dtb)
        check_root
        check_output
        install_dtb
        ;;

    modules)
        check_root
        check_output
        install_modules
        ;;

    drivers)
        check_root
        install_drivers
        ;;

    clean)
        clean_install
        ;;

    help)
        show_help
        ;;

    *)
        error "Unknown option: $1"
        show_help
        exit 1
        ;;

esac
