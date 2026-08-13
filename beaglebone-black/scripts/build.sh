#!/bin/bash

###############################################################################
# BeagleBone Black - Complete Device Driver Project
#
# Build Script
#
# Builds:
#   1. Linux Kernel
#   2. Device Tree
#   3. Kernel Modules
#   4. Device Driver Components
#
# Target:
#   BeagleBone Black / AM335x
#
# Usage:
#   ./scripts/build.sh
#   ./scripts/build.sh clean
#   ./scripts/build.sh kernel
#   ./scripts/build.sh dtb
#   ./scripts/build.sh modules
#   ./scripts/build.sh all
###############################################################################

set -e

###############################################################################
# Project Paths
###############################################################################

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

KERNEL_DIR="${PROJECT_ROOT}/kernel/linux"
DEVICE_TREE_DIR="${PROJECT_ROOT}/device-tree"
DRIVER_DIR="${PROJECT_ROOT}/drivers"
BUILD_DIR="${PROJECT_ROOT}/build"
OUTPUT_DIR="${PROJECT_ROOT}/output"

###############################################################################
# Kernel Configuration
###############################################################################

KERNEL_CONFIG="${PROJECT_ROOT}/kernel/config/bbb_driver_defconfig"

###############################################################################
# Architecture / Cross Compiler
###############################################################################

ARCH="${ARCH:-arm}"

CROSS_COMPILE="${CROSS_COMPILE:-arm-linux-gnueabihf-}"

export ARCH
export CROSS_COMPILE

###############################################################################
# CPU Build Jobs
###############################################################################

JOBS="${JOBS:-$(nproc)}"

###############################################################################
# Colors
###############################################################################

GREEN="\033[0;32m"
YELLOW="\033[1;33m"
RED="\033[0;31m"
NC="\033[0m"

###############################################################################
# Functions
###############################################################################

print_info()
{
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning()
{
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error()
{
    echo -e "${RED}[ERROR]${NC} $1"
}

check_kernel()
{
    if [ ! -d "${KERNEL_DIR}" ]; then
        print_error "Linux kernel source not found:"
        echo "${KERNEL_DIR}"
        exit 1
    fi
}

prepare_directories()
{
    print_info "Preparing build directories..."

    mkdir -p "${BUILD_DIR}"
    mkdir -p "${OUTPUT_DIR}"
}

###############################################################################
# Kernel Configuration
###############################################################################

configure_kernel()
{
    print_info "Configuring Linux kernel..."

    check_kernel

    if [ ! -f "${KERNEL_CONFIG}" ]; then
        print_error "Kernel configuration not found:"
        echo "${KERNEL_CONFIG}"
        exit 1
    fi

    cp "${KERNEL_CONFIG}" "${KERNEL_DIR}/.config"

    cd "${KERNEL_DIR}"

    make ARCH="${ARCH}" \
         CROSS_COMPILE="${CROSS_COMPILE}" \
         olddefconfig

    print_info "Kernel configuration completed."
}

###############################################################################
# Build Linux Kernel
###############################################################################

build_kernel()
{
    print_info "Building Linux kernel..."

    check_kernel

    cd "${KERNEL_DIR}"

    make ARCH="${ARCH}" \
         CROSS_COMPILE="${CROSS_COMPILE}" \
         -j"${JOBS}" \
         zImage

    print_info "Linux kernel build completed."

    if [ -f "arch/arm/boot/zImage" ]; then
        cp arch/arm/boot/zImage "${OUTPUT_DIR}/"
    fi
}

###############################################################################
# Build Device Tree
###############################################################################

build_dtb()
{
    print_info "Building Device Tree..."

    check_kernel

    cd "${KERNEL_DIR}"

    make ARCH="${ARCH}" \
         CROSS_COMPILE="${CROSS_COMPILE}" \
         -j"${JOBS}" \
         dtbs

    print_info "Device Tree build completed."

    mkdir -p "${OUTPUT_DIR}/dtbs"

    find arch/arm/boot/dts \
        -name "*.dtb" \
        -exec cp {} "${OUTPUT_DIR}/dtbs/" \;

    find arch/arm/boot/dts \
        -name "*.dtbo" \
        -exec cp {} "${OUTPUT_DIR}/dtbs/" \; 2>/dev/null || true
}

###############################################################################
# Build Kernel Modules
###############################################################################

build_modules()
{
    print_info "Building kernel modules..."

    check_kernel

    cd "${KERNEL_DIR}"

    make ARCH="${ARCH}" \
         CROSS_COMPILE="${CROSS_COMPILE}" \
         -j"${JOBS}" \
         modules

    print_info "Kernel modules build completed."
}

###############################################################################
# Install Modules
###############################################################################

install_modules()
{
    print_info "Installing kernel modules into output directory..."

    check_kernel

    mkdir -p "${OUTPUT_DIR}/modules"

    cd "${KERNEL_DIR}"

    make ARCH="${ARCH}" \
         CROSS_COMPILE="${CROSS_COMPILE}" \
         INSTALL_MOD_PATH="${OUTPUT_DIR}/modules" \
         modules_install

    print_info "Kernel modules installed."
}

###############################################################################
# Build All
###############################################################################

build_all()
{
    print_info "Starting complete BeagleBone Black build..."

    prepare_directories

    configure_kernel
    build_kernel
    build_dtb
    build_modules
    install_modules

    print_info "=========================================="
    print_info "Complete build successful!"
    print_info "=========================================="

    echo
    echo "Output:"
    echo "  ${OUTPUT_DIR}/zImage"
    echo "  ${OUTPUT_DIR}/dtbs/"
    echo "  ${OUTPUT_DIR}/modules/"
}

###############################################################################
# Clean
###############################################################################

clean_build()
{
    print_warning "Cleaning build artifacts..."

    rm -rf "${BUILD_DIR}"
    rm -rf "${OUTPUT_DIR}"

    if [ -d "${KERNEL_DIR}" ]; then
        cd "${KERNEL_DIR}"

        make ARCH="${ARCH}" \
             CROSS_COMPILE="${CROSS_COMPILE}" \
             clean
    fi

    print_info "Clean completed."
}

###############################################################################
# Help
###############################################################################

show_help()
{
    echo
    echo "BeagleBone Black Device Driver Build System"
    echo
    echo "Usage:"
    echo
    echo "  ./scripts/build.sh all"
    echo "      Build kernel, DTB and modules"
    echo
    echo "  ./scripts/build.sh kernel"
    echo "      Build Linux kernel"
    echo
    echo "  ./scripts/build.sh dtb"
    echo "      Build Device Tree"
    echo
    echo "  ./scripts/build.sh modules"
    echo "      Build kernel modules"
    echo
    echo "  ./scripts/build.sh config"
    echo "      Configure kernel"
    echo
    echo "  ./scripts/build.sh install-modules"
    echo "      Install kernel modules"
    echo
    echo "  ./scripts/build.sh clean"
    echo "      Clean build artifacts"
    echo
    echo "  ./scripts/build.sh help"
    echo "      Show this help"
    echo
}

###############################################################################
# Main
###############################################################################

case "$1" in

    all)
        build_all
        ;;

    kernel)
        prepare_directories
        configure_kernel
        build_kernel
        ;;

    dtb)
        prepare_directories
        build_dtb
        ;;

    modules)
        prepare_directories
        configure_kernel
        build_modules
        ;;

    config)
        configure_kernel
        ;;

    install-modules)
        install_modules
        ;;

    clean)
        clean_build
        ;;

    help|"")
        show_help
        ;;

    *)
        print_error "Unknown command: $1"
        show_help
        exit 1
        ;;

esac
