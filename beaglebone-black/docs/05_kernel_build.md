# `05_kernel_build.md`

Create:

```text
beaglebone-black/docs/05_kernel_build.md
```

````markdown
# 05 - Linux Kernel Build for BeagleBone Black

## Table of Contents

- [1. Overview](#1-overview)
- [2. Why Build the Kernel](#2-why-build-the-kernel)
- [3. Kernel Build Architecture](#3-kernel-build-architecture)
- [4. Host Machine Setup](#4-host-machine-setup)
- [5. Required Packages](#5-required-packages)
- [6. Clone Linux Kernel Source](#6-clone-linux-kernel-source)
- [7. Kernel Source Tree](#7-kernel-source-tree)
- [8. Select BeagleBone Black Configuration](#8-select-beaglebone-black-configuration)
- [9. Configure the Kernel](#9-configure-the-kernel)
- [10. Kernel Configuration Options](#10-kernel-configuration-options)
- [11. Build the Linux Kernel](#11-build-the-linux-kernel)
- [12. Build Device Tree](#12-build-device-tree)
- [13. Build Kernel Modules](#13-build-kernel-modules)
- [14. Install Kernel Modules](#14-install-kernel-modules)
- [15. Kernel Build Outputs](#15-kernel-build-outputs)
- [16. Build with Out-of-Tree Directory](#16-build-with-out-of-tree-directory)
- [17. Build Only Device Trees](#17-build-only-device-trees)
- [18. Build Only Kernel Modules](#18-build-only-kernel-modules)
- [19. Build a Custom Driver](#19-build-a-custom-driver)
- [20. Kernel and Device Tree Relationship](#20-kernel-and-device-tree-relationship)
- [21. Booting the Custom Kernel](#21-booting-the-custom-kernel)
- [22. Copy Kernel to SD Card](#22-copy-kernel-to-sd-card)
- [23. Copy Device Tree to SD Card](#23-copy-device-tree-to-sd-card)
- [24. Boot Verification](#24-boot-verification)
- [25. Verify Kernel Version](#25-verify-kernel-version)
- [26. Verify Kernel Configuration](#26-verify-kernel-configuration)
- [27. Verify Device Tree](#27-verify-device-tree)
- [28. Verify Drivers](#28-verify-drivers)
- [29. Kernel Logs](#29-kernel-logs)
- [30. Debugging Kernel Build Errors](#30-debugging-kernel-build-errors)
- [31. Common Build Errors](#31-common-build-errors)
- [32. Clean Kernel Build](#32-clean-kernel-build)
- [33. Rebuild Workflow](#33-rebuild-workflow)
- [34. Kernel Driver Development Workflow](#34-kernel-driver-development-workflow)
- [35. Project Integration](#35-project-integration)
- [36. Recommended Build Script](#36-recommended-build-script)
- [37. Interview Explanation](#37-interview-explanation)
- [38. Final Checklist](#38-final-checklist)
- [39. Summary](#39-summary)

---

# 1. Overview

The Linux kernel is the main software component responsible for
managing the hardware of the BeagleBone Black.

In this project, the kernel provides support for:

```text
GPIO
UART
I2C
SPI
PWM
ADC
CAN
USB
Ethernet
MMC/SD
Storage
Interrupts
DMA
Clocks
Power Management
````

The kernel build process converts the Linux kernel source code into
bootable and loadable components.

The basic flow is:

```text
Linux Kernel Source
        |
        v
Kernel Configuration
        |
        v
Compilation
        |
        +----------------+
        |                |
        v                v
      Kernel           Device Tree
        |                |
        v                v
     Image             DTB
        |                |
        +-------+--------+
                |
                v
             U-Boot
                |
                v
          Linux Kernel
                |
                v
          Device Drivers
```

---

# 2. Why Build the Kernel

An Embedded Linux engineer may need to build the kernel to:

* Enable a peripheral
* Disable an unused driver
* Add a custom device driver
* Modify an existing driver
* Change Device Tree configuration
* Enable debugging
* Enable tracing
* Enable networking features
* Enable filesystem support
* Enable security features
* Modify kernel configuration
* Support new hardware
* Fix a kernel bug

For this project, kernel customization is important because the
BeagleBone Black is being used as a complete Embedded Linux driver
development platform.

---

# 3. Kernel Build Architecture

The complete build process is:

```text
+---------------------------+
| Linux Kernel Source       |
|                           |
| arch/                     |
| drivers/                  |
| include/                  |
| kernel/                   |
| mm/                       |
| fs/                       |
| net/                      |
+-------------+-------------+
              |
              v
+---------------------------+
| Kernel Configuration      |
|                           |
| make menuconfig           |
| make olddefconfig         |
+-------------+-------------+
              |
              v
+---------------------------+
| Cross Compiler            |
|                           |
| arm-linux-gnueabihf-gcc   |
+-------------+-------------+
              |
              v
+---------------------------+
| Kernel Build              |
+-------------+-------------+
              |
       +------+------+
       |             |
       v             v
   zImage          DTB
       |             |
       +------+------+
              |
              v
          Bootloader
              |
              v
            Linux
```

---

# 4. Host Machine Setup

The kernel should normally be built on a development workstation
rather than directly on the BeagleBone Black.

Example host:

```text
Ubuntu Linux
```

Check:

```bash
uname -a
```

Check architecture:

```bash
uname -m
```

Example:

```text
x86_64
```

The target architecture is:

```text
ARM
```

Therefore, a cross compiler is normally used.

---

# 5. Required Packages

Install common kernel build dependencies:

```bash
sudo apt update
```

```bash
sudo apt install -y \
    git \
    build-essential \
    bc \
    bison \
    flex \
    libssl-dev \
    libelf-dev \
    libncurses-dev \
    device-tree-compiler \
    crossbuild-essential-armhf
```

Verify GCC:

```bash
gcc --version
```

Verify cross compiler:

```bash
arm-linux-gnueabihf-gcc --version
```

Verify Device Tree Compiler:

```bash
dtc --version
```

---

# 6. Clone Linux Kernel Source

Create a workspace:

```bash
mkdir -p ~/beaglebone-linux
cd ~/beaglebone-linux
```

Clone the Linux kernel source:

```bash
git clone https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git
```

Enter the source tree:

```bash
cd linux
```

Check the available branches/tags:

```bash
git branch -a
```

Check kernel version:

```bash
make kernelversion
```

---

# 7. Kernel Source Tree

Important directories:

```text
linux/
|
+-- arch/
|   +-- arm/
|
+-- drivers/
|   +-- gpio/
|   +-- i2c/
|   +-- spi/
|   +-- pwm/
|   +-- tty/
|   +-- net/
|       +-- can/
|
+-- include/
|
+-- kernel/
|
+-- mm/
|
+-- fs/
|
+-- net/
|
+-- Documentation/
|
+-- scripts/
|
+-- Makefile
```

For this project, the most important areas are:

```text
drivers/gpio/
drivers/i2c/
drivers/spi/
drivers/pwm/
drivers/tty/
drivers/iio/
drivers/net/can/
arch/arm/
arch/arm/boot/dts/
```

---

# 8. Select BeagleBone Black Configuration

The Linux kernel provides a board-specific default configuration for
BeagleBone Black.

Depending on the kernel version, the configuration target may be
available as:

```bash
make omap2plus_defconfig
```

For a 32-bit ARM BeagleBone Black kernel:

```bash
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- omap2plus_defconfig
```

This creates:

```text
.config
```

Verify:

```bash
ls -l .config
```

---

# 9. Configure the Kernel

After selecting the default configuration, customize it.

Use:

```bash
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- menuconfig
```

You will see:

```text
+--------------------------------------+
| Linux Kernel Configuration           |
|                                      |
| General setup                        |
| Device Drivers                       |
| File systems                          |
| Networking support                   |
| Kernel hacking                       |
| Security options                     |
|                                      |
+--------------------------------------+
```

Save the configuration.

The result is:

```text
.config
```

---

# 10. Kernel Configuration Options

Kernel configuration uses:

```text
CONFIG_*
```

For example:

```text
CONFIG_I2C=y
CONFIG_SPI=y
CONFIG_GPIO_SYSFS=y
CONFIG_CAN=y
CONFIG_PWM=y
```

Possible configuration states:

```text
y = built into kernel
m = kernel module
n = disabled
```

Example:

```text
CONFIG_I2C=y
```

means the I2C support is built into the kernel.

Example:

```text
CONFIG_I2C=m
```

means the I2C support is built as a module.

Example:

```text
CONFIG_I2C=n
```

means it is disabled.

---

# 11. Build the Linux Kernel

Build the kernel:

```bash
make -j$(nproc) \
    ARCH=arm \
    CROSS_COMPILE=arm-linux-gnueabihf-
```

The number of parallel jobs can be checked with:

```bash
nproc
```

For example:

```bash
make -j8 ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
```

Build process:

```text
.config
   |
   v
Compiler
   |
   v
C source files
   |
   v
Object files
   |
   v
Kernel image
```

---

# 12. Build Device Tree

Device Tree files are also built as part of the kernel build.

Build only the Device Trees:

```bash
make ARCH=arm \
     CROSS_COMPILE=arm-linux-gnueabihf- \
     dtbs
```

Kernel Device Trees are normally located under:

```text
arch/arm/boot/dts/
```

The exact filename depends on the kernel version and DTS naming
scheme.

Search for BeagleBone files:

```bash
find arch/arm/boot/dts/ -iname "*bone*"
```

---

# 13. Build Kernel Modules

Build modules:

```bash
make -j$(nproc) \
    ARCH=arm \
    CROSS_COMPILE=arm-linux-gnueabihf- \
    modules
```

Kernel modules normally have:

```text
.ko
```

extension.

Example:

```text
my_driver.ko
```

---

# 14. Install Kernel Modules

Create a target root filesystem directory:

```bash
mkdir -p ~/beaglebone-linux/rootfs
```

Install modules:

```bash
make ARCH=arm \
     CROSS_COMPILE=arm-linux-gnueabihf- \
     modules_install \
     INSTALL_MOD_PATH=~/beaglebone-linux/rootfs
```

The modules will be installed under:

```text
rootfs/lib/modules/<kernel-version>/
```

---

# 15. Kernel Build Outputs

Important output files include:

```text
arch/arm/boot/zImage
```

and:

```text
arch/arm/boot/dts/*.dtb
```

Kernel modules:

```text
*.ko
```

Kernel configuration:

```text
.config
```

Map file:

```text
System.map
```

Kernel image:

```text
vmlinux
```

Important:

```text
vmlinux
    |
    +-- ELF kernel image
    |
    +-- debugging symbols
```

Boot image:

```text
zImage
```

Device Tree:

```text
*.dtb
```

---

# 16. Build with Out-of-Tree Directory

A cleaner development method is to keep generated build files outside
the source tree.

Create:

```bash
mkdir -p ~/beaglebone-linux/build
```

Build using:

```bash
make O=~/beaglebone-linux/build \
     ARCH=arm \
     CROSS_COMPILE=arm-linux-gnueabihf- \
     omap2plus_defconfig
```

Configure:

```bash
make O=~/beaglebone-linux/build \
     ARCH=arm \
     CROSS_COMPILE=arm-linux-gnueabihf- \
     menuconfig
```

Build:

```bash
make -j$(nproc) \
     O=~/beaglebone-linux/build \
     ARCH=arm \
     CROSS_COMPILE=arm-linux-gnueabihf-
```

Now:

```text
linux/
|
+-- Source code
|
+-- arch/
+-- drivers/
+-- include/
|
build/
|
+-- .config
+-- vmlinux
+-- arch/
+-- drivers/
```

This keeps the source tree cleaner.

---

# 17. Build Only Device Trees

If you changed only a Device Tree file:

```bash
make O=~/beaglebone-linux/build \
     ARCH=arm \
     CROSS_COMPILE=arm-linux-gnueabihf- \
     dtbs
```

This is much faster than rebuilding everything.

---

# 18. Build Only Kernel Modules

If only a driver changed:

```bash
make O=~/beaglebone-linux/build \
     ARCH=arm \
     CROSS_COMPILE=arm-linux-gnueabihf- \
     modules
```

For a specific module:

```bash
make O=~/beaglebone-linux/build \
     ARCH=arm \
     CROSS_COMPILE=arm-linux-gnueabihf- \
     M=drivers/my_driver \
     modules
```

---

# 19. Build a Custom Driver

For this project, custom drivers should be placed separately from the
standard kernel subsystem drivers when appropriate.

Example:

```text
beaglebone-black/
|
+-- drivers/
|   |
|   +-- gpio/
|   +-- i2c/
|   +-- spi/
|   +-- uart/
|   +-- pwm/
|   +-- adc/
|   +-- can/
|   |
|   +-- custom/
|       +-- bbb_demo.c
|       +-- Makefile
|       +-- Kconfig
```

Example driver:

```c
static int bbb_demo_probe(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "BBB driver probed\n");

    return 0;
}

static int bbb_demo_remove(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "BBB driver removed\n");

    return 0;
}
```

The Device Tree can describe the device:

```dts
demo_device {
    compatible = "bbb,demo-device";
    status = "okay";
};
```

Driver matching:

```text
Device Tree
     |
     | compatible
     v
"bbb,demo-device"
     |
     v
Driver match table
     |
     v
probe()
```

---

# 20. Kernel and Device Tree Relationship

Kernel source and Device Tree are separate but connected.

```text
                 Linux Kernel
                      |
       +--------------+--------------+
       |                             |
       v                             v
   Driver Code                  Device Tree
       |                             |
       |                             |
       +-------------+---------------+
                     |
                     v
              Hardware Device
```

For example:

```text
bbb-i2c.dts
      |
      v
I2C Device Description
      |
      v
I2C Framework
      |
      v
I2C Driver
```

The Device Tree does not implement the driver logic.

---

# 21. Booting the Custom Kernel

The boot process is generally:

```text
Boot ROM
   |
   v
SPL
   |
   v
U-Boot
   |
   +---- Kernel
   |
   +---- Device Tree
   |
   v
Linux
```

The exact file names and boot environment depend on the Linux image
and U-Boot configuration installed on the BeagleBone Black.

Before replacing a working kernel, keep a known-good kernel available
so the board can be recovered if the new kernel fails.

---

# 22. Copy Kernel to SD Card

Identify the SD card:

```bash
lsblk
```

Example:

```text
sdb
├── sdb1
└── sdb2
```

Mount the boot partition:

```bash
sudo mount /dev/sdb1 /mnt/bbb-boot
```

Copy the kernel:

```bash
sudo cp ~/beaglebone-linux/build/arch/arm/boot/zImage \
    /mnt/bbb-boot/
```

The actual boot file naming convention depends on the image.

Do not blindly overwrite the production kernel.

A safer approach is to use a separate filename where the bootloader
configuration supports it.

---

# 23. Copy Device Tree to SD Card

Build DTBs:

```bash
make O=~/beaglebone-linux/build \
     ARCH=arm \
     CROSS_COMPILE=arm-linux-gnueabihf- \
     dtbs
```

Find the BeagleBone DTBs:

```bash
find ~/beaglebone-linux/build/arch/arm/boot/dts/ \
    -iname "*bone*.dtb"
```

Copy the required DTB:

```bash
sudo cp <beaglebone-dtb> /mnt/bbb-boot/
```

The DTB selected by U-Boot must correspond to the board and kernel
configuration.

---

# 24. Boot Verification

Connect the BeagleBone Black serial console.

Typical workflow:

```text
PC
 |
 +---- USB/UART
 |
 v
BeagleBone Black
```

Open a serial terminal using a suitable application.

For example:

```bash
screen /dev/ttyUSB0 115200
```

The actual serial device may be different:

```bash
ls /dev/ttyUSB*
ls /dev/ttyACM*
```

Power the board.

Expected boot flow:

```text
U-Boot
  |
  v
Loading Kernel
  |
  v
Loading Device Tree
  |
  v
Starting Kernel
  |
  v
Linux Boot Messages
  |
  v
Login Prompt
```

---

# 25. Verify Kernel Version

On the BeagleBone:

```bash
uname -a
```

or:

```bash
uname -r
```

Example:

```text
6.x.x-custom
```

This allows you to confirm that the expected kernel is running.

---

# 26. Verify Kernel Configuration

If the kernel exposes its configuration:

```bash
zcat /proc/config.gz | grep CONFIG_I2C
```

If `/proc/config.gz` is unavailable:

```bash
cat /boot/config-$(uname -r) | grep CONFIG_I2C
```

Check:

```bash
cat /boot/config-$(uname -r) | grep CONFIG_SPI
```

```bash
cat /boot/config-$(uname -r) | grep CONFIG_CAN
```

```bash
cat /boot/config-$(uname -r) | grep CONFIG_PWM
```

---

# 27. Verify Device Tree

Check:

```bash
ls /sys/firmware/devicetree/base/
```

Check model:

```bash
tr -d '\0' < /sys/firmware/devicetree/base/model
echo
```

Check compatible:

```bash
tr '\0' '\n' < /sys/firmware/devicetree/base/compatible
```

The exact node path varies according to the Device Tree.

---

# 28. Verify Drivers

Check loaded modules:

```bash
lsmod
```

Check kernel modules:

```bash
find /lib/modules/$(uname -r) -name "*.ko*"
```

Check platform drivers:

```bash
ls /sys/bus/platform/drivers/
```

Check I2C drivers:

```bash
ls /sys/bus/i2c/drivers/
```

Check SPI drivers:

```bash
ls /sys/bus/spi/drivers/
```

Check GPIO:

```bash
ls /sys/class/gpio/
```

Modern GPIO development should primarily use the descriptor-based GPIO
API and GPIO character-device interfaces rather than relying on the
deprecated legacy sysfs GPIO interface.

---

# 29. Kernel Logs

Kernel logs are one of the most important debugging tools.

View all messages:

```bash
dmesg
```

Follow recent messages:

```bash
dmesg | tail -50
```

Search for a peripheral:

```bash
dmesg | grep -i gpio
```

```bash
dmesg | grep -i uart
```

```bash
dmesg | grep -i i2c
```

```bash
dmesg | grep -i spi
```

```bash
dmesg | grep -i pwm
```

```bash
dmesg | grep -i can
```

For a custom driver:

```bash
dmesg | grep -i bbb
```

---

# 30. Debugging Kernel Build Errors

When the build fails, first capture the actual error:

```bash
make ...
```

Do not focus on the final:

```text
Error 2
```

Instead find the first meaningful compiler or linker error.

For example:

```text
error: implicit declaration of function ...
```

or:

```text
fatal error: xxx.h: No such file or directory
```

or:

```text
undefined reference to ...
```

Debug workflow:

```text
Build Failure
     |
     v
Find First Real Error
     |
     v
Check Source
     |
     v
Check Kconfig
     |
     v
Check Makefile
     |
     v
Check Dependencies
     |
     v
Rebuild
```

---

# 31. Common Build Errors

## Error 1 - Cross Compiler Not Found

```text
arm-linux-gnueabihf-gcc: command not found
```

Install:

```bash
sudo apt install crossbuild-essential-armhf
```

Check:

```bash
arm-linux-gnueabihf-gcc --version
```

---

## Error 2 - Missing ncurses

Menuconfig may fail because the required development library is
missing.

Install:

```bash
sudo apt install libncurses-dev
```

---

## Error 3 - Missing Bison/Flex

Install:

```bash
sudo apt install bison flex
```

---

## Error 4 - Missing OpenSSL Headers

Install:

```bash
sudo apt install libssl-dev
```

---

## Error 5 - Missing ELF Headers

Install:

```bash
sudo apt install libelf-dev
```

---

## Error 6 - Device Tree Compiler Missing

Check:

```bash
dtc --version
```

Install:

```bash
sudo apt install device-tree-compiler
```

---

## Error 7 - Wrong Architecture

Check:

```bash
make ARCH=arm ...
```

For the 32-bit BeagleBone Black kernel.

---

## Error 8 - Wrong Cross Compiler

Use:

```bash
CROSS_COMPILE=arm-linux-gnueabihf-
```

The compiler prefix must match the target ABI and kernel/userspace
configuration.

---

# 32. Clean Kernel Build

To remove most generated files:

```bash
make clean
```

To perform a more extensive cleanup:

```bash
make mrproper
```

Be careful with:

```bash
make mrproper
```

because it removes the kernel configuration as well.

If you need to preserve `.config`, back it up first:

```bash
cp .config ~/bbb-kernel.config
```

After `mrproper`:

```bash
cp ~/bbb-kernel.config .config
```

---

# 33. Rebuild Workflow

For normal development:

```text
Modify Source
     |
     v
make
     |
     v
Build
     |
     v
Copy Kernel/Module
     |
     v
Boot Board
     |
     v
dmesg
     |
     v
Test
```

If only Device Tree changed:

```text
Modify DTS
    |
    v
make dtbs
    |
    v
Copy DTB
    |
    v
Reboot
    |
    v
Test
```

If only a module changed:

```text
Modify Driver
    |
    v
make M=<driver>
    |
    v
Copy .ko
    |
    v
insmod/modprobe
    |
    v
dmesg
```

---

# 34. Kernel Driver Development Workflow

This project should follow this workflow for every peripheral:

```text
                Hardware
                   |
                   v
             Device Tree
                   |
                   v
            Kernel Config
                   |
                   v
             Driver Code
                   |
                   v
               Compile
                   |
                   v
                Install
                   |
                   v
                 Boot
                   |
                   v
             Driver Match
                   |
                   v
                probe()
                   |
                   v
           Hardware Init
                   |
                   v
             Kernel Logs
                   |
                   v
              Test Tool
                   |
                   v
            Hardware Test
```

---

# 35. Project Integration

The kernel build should integrate with this project structure:

```text
beaglebone-black/
|
+-- docs/
|   |
|   +-- 01_architecture.md
|   +-- 02_hardware_setup.md
|   +-- 03_linux_driver_model.md
|   +-- 04_device_tree.md
|   +-- 05_kernel_build.md
|
+-- device-tree/
|   |
|   +-- adc/
|   +-- can/
|   +-- gpio/
|   +-- i2c/
|   +-- overlays/
|   +-- pwm/
|   +-- spi/
|   +-- uart/
|
+-- drivers/
|   |
|   +-- gpio/
|   +-- i2c/
|   +-- spi/
|   +-- uart/
|   +-- pwm/
|   +-- adc/
|   +-- can/
|
+-- tests/
|
+-- scripts/
|
+-- README.md
```

Recommended responsibilities:

```text
device-tree/
    Hardware description

drivers/
    Linux driver implementation

tests/
    Hardware/software validation

scripts/
    Build and deployment automation

docs/
    Technical documentation
```

---

# 36. Recommended Build Script

Create:

```text
scripts/build_kernel.sh
```

Example:

```bash
#!/bin/bash

set -e

KERNEL_SRC="${KERNEL_SRC:-$HOME/beaglebone-linux/linux}"
BUILD_DIR="${BUILD_DIR:-$HOME/beaglebone-linux/build}"

ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-

echo "======================================"
echo " BeagleBone Black Kernel Build"
echo "======================================"

echo "[1/4] Kernel source:"
echo "$KERNEL_SRC"

echo "[2/4] Build directory:"
echo "$BUILD_DIR"

mkdir -p "$BUILD_DIR"

echo "[3/4] Configuring kernel..."

make -C "$KERNEL_SRC" \
    O="$BUILD_DIR" \
    ARCH="$ARCH" \
    CROSS_COMPILE="$CROSS_COMPILE" \
    omap2plus_defconfig

echo "[4/4] Building kernel..."

make -C "$KERNEL_SRC" \
    O="$BUILD_DIR" \
    ARCH="$ARCH" \
    CROSS_COMPILE="$CROSS_COMPILE" \
    -j"$(nproc)"

echo
echo "Kernel build completed."
echo
echo "Kernel:"
echo "$BUILD_DIR/arch/arm/boot/zImage"

echo
echo "Device Trees:"
find "$BUILD_DIR/arch/arm/boot/dts" \
    -iname "*bone*.dtb" 2>/dev/null || true
```

Make executable:

```bash
chmod +x scripts/build_kernel.sh
```

Run:

```bash
./scripts/build_kernel.sh
```

---

# 37. Recommended DTB Build Script

Create:

```text
scripts/build_dtbs.sh
```

```bash
#!/bin/bash

set -e

KERNEL_SRC="${KERNEL_SRC:-$HOME/beaglebone-linux/linux}"
BUILD_DIR="${BUILD_DIR:-$HOME/beaglebone-linux/build}"

make -C "$KERNEL_SRC" \
    O="$BUILD_DIR" \
    ARCH=arm \
    CROSS_COMPILE=arm-linux-gnueabihf- \
    dtbs

echo
echo "Device Tree build completed."
echo

find "$BUILD_DIR/arch/arm/boot/dts" \
    -iname "*.dtb"
```

Run:

```bash
chmod +x scripts/build_dtbs.sh
./scripts/build_dtbs.sh
```

---

# 38. Recommended Driver Build Strategy

For this project, use two approaches.

## Approach 1 - In-tree driver

Use this when the driver is intended to become part of the kernel
source tree.

```text
drivers/
   |
   +-- custom/
       |
       +-- bbb_driver.c
       +-- Kconfig
       +-- Makefile
```

Advantages:

```text
Kernel integration
Kconfig support
Device Tree integration
Normal kernel build
```

---

## Approach 2 - Out-of-tree module

Use this during early driver development.

```text
bbb-driver/
|
+-- bbb_driver.c
+-- Makefile
```

Build:

```bash
make -C ~/beaglebone-linux/linux \
    M=$PWD \
    ARCH=arm \
    CROSS_COMPILE=arm-linux-gnueabihf- \
    modules
```

Output:

```text
bbb_driver.ko
```

Copy:

```bash
scp bbb_driver.ko debian@<board-ip>:/tmp/
```

On the board:

```bash
sudo insmod /tmp/bbb_driver.ko
```

Check:

```bash
dmesg | tail
```

Remove:

```bash
sudo rmmod bbb_driver
```

---

# 39. Kernel Configuration for This Project

The project should enable the major Linux subsystems required by the
peripheral drivers.

Conceptually:

```text
CONFIG_GPIO
CONFIG_I2C
CONFIG_SPI
CONFIG_PWM
CONFIG_IIO
CONFIG_CAN
CONFIG_SERIAL
```

For CAN:

```text
CONFIG_CAN
CONFIG_CAN_DEV
```

For I2C:

```text
CONFIG_I2C
```

For SPI:

```text
CONFIG_SPI
```

For IIO:

```text
CONFIG_IIO
```

For PWM:

```text
CONFIG_PWM
```

The exact individual controller-driver options depend on the kernel
version and SoC support.

Use:

```bash
make menuconfig
```

to inspect the options available in your selected kernel version.

---

# 40. Kernel Configuration Verification

After configuring:

```bash
grep "^CONFIG_I2C" .config
```

```bash
grep "^CONFIG_SPI" .config
```

```bash
grep "^CONFIG_PWM" .config
```

```bash
grep "^CONFIG_IIO" .config
```

```bash
grep "^CONFIG_CAN" .config
```

```bash
grep "^CONFIG_SERIAL" .config
```

This is useful before starting a long kernel build.

---

# 41. Kernel Build Performance

Check CPU cores:

```bash
nproc
```

Build in parallel:

```bash
make -j$(nproc)
```

For a machine with 8 logical CPUs:

```bash
make -j8
```

Parallel compilation significantly reduces build time.

Do not use an excessively high job count on a system with limited
RAM.

---

# 42. Reproducible Kernel Configuration

Save the project configuration:

```bash
cp ~/beaglebone-linux/build/.config \
   configs/beaglebone-black.config
```

Recommended project structure:

```text
beaglebone-black/
|
+-- configs/
|   |
|   +-- beaglebone-black.config
|
+-- docs/
+-- device-tree/
+-- drivers/
+-- tests/
+-- scripts/
```

This allows another developer to reproduce the kernel configuration.

---

# 43. Git Management

Do not normally commit the entire Linux kernel source tree into this
project.

Instead, track:

```text
Kernel version
Kernel configuration
Device Tree changes
Custom drivers
Build scripts
Documentation
```

Example:

```text
beaglebone-black/
|
+-- configs/
|   +-- beaglebone-black.config
|
+-- device-tree/
|
+-- drivers/
|
+-- scripts/
|
+-- docs/
```

The external Linux kernel source can be downloaded using Git.

---

# 44. Recommended Git Workflow

Create a branch:

```bash
git checkout -b feature/kernel-build
```

After making changes:

```bash
git status
```

Review:

```bash
git diff
```

Commit:

```bash
git add docs/05_kernel_build.md
```

```bash
git commit -m "docs: add BeagleBone Black kernel build guide"
```

---

# 45. Full Build and Deployment Flow

The complete project flow is:

```text
                    Git Repository
                          |
                          v
                 Linux Kernel Source
                          |
                          v
                 Kernel Configuration
                          |
                          v
                 Device Tree Changes
                          |
                          v
                   Driver Changes
                          |
                          v
                      Build
                          |
          +---------------+---------------+
          |               |               |
          v               v               v
       zImage           DTB              .ko
          |               |               |
          +---------------+---------------+
                          |
                          v
                    SD Card / Boot
                          |
                          v
                    BeagleBone Black
                          |
                          v
                       U-Boot
                          |
                          v
                       Linux
                          |
                          v
                   Driver Probe
                          |
                          v
                    Hardware Test
```

---

# 46. Peripheral Build/Test Matrix

| Peripheral | Device Tree    | Kernel Subsystem | Driver      | Basic Validation    |
| ---------- | -------------- | ---------------- | ----------- | ------------------- |
| GPIO       | `bbb-gpio.dts` | GPIO             | GPIO driver | GPIO state test     |
| UART       | `bbb-uart.dts` | TTY/Serial       | UART driver | Serial console/data |
| I2C        | `bbb-i2c.dts`  | I2C              | I2C driver  | Bus/device test     |
| SPI        | `bbb-spi.dts`  | SPI              | SPI driver  | SPI transfer        |
| PWM        | `bbb-pwm.dts`  | PWM              | PWM driver  | Duty/period test    |
| ADC        | `bbb-adc.dts`  | IIO              | ADC driver  | ADC raw value       |
| CAN        | `bbb-can.dts`  | SocketCAN        | CAN driver  | CAN TX/RX           |

The exact DT node names and kernel driver names vary by kernel/BSP
version.

---

# 47. Kernel Debugging Tools

Important tools for this project:

```text
dmesg
journalctl
lsmod
modprobe
insmod
rmmod
cat /proc/config.gz
sysfs
debugfs
ftrace
perf
gdb
kgdb
JTAG
```

For basic driver development:

```bash
dmesg
```

is one of the first tools to use.

For module management:

```bash
modprobe
insmod
rmmod
```

For runtime Device Tree:

```text
/sys/firmware/devicetree/base/
```

For devices:

```text
/sys/bus/
```

---

# 48. Kernel Build Debugging Flow

When a peripheral does not work:

```text
                     Peripheral Failure
                             |
                             v
                       Check dmesg
                             |
                             v
                     Check Device Tree
                             |
                             v
                    Check kernel config
                             |
                             v
                     Check driver match
                             |
                             v
                      Check probe()
                             |
                             v
                    Check pinmux/GPIO
                             |
                             v
                    Check hardware wiring
                             |
                             v
                         Test again
```

This workflow should be used consistently throughout the project.

---

# 49. Interview Explanation

A strong interview answer:

> "For the BeagleBone Black, I use a cross-compilation environment to
> build the ARM Linux kernel from source. I first select the
> appropriate board configuration, customize the kernel using
> menuconfig, and enable the required subsystems such as GPIO, I2C,
> SPI, PWM, IIO, CAN and serial support. I then build the kernel,
> Device Tree blobs and kernel modules using the ARM cross compiler.
> After deployment to the SD card, U-Boot loads the kernel and Device
> Tree. During boot, Linux creates devices from the Device Tree and
> matches them with the appropriate drivers. I validate the result
> using dmesg, sysfs, subsystem tools and hardware-level tests."

---

# 50. 30-Second Project Explanation

> "I am building a complete Embedded Linux device-driver development
> platform using the BeagleBone Black. I maintain the Linux kernel
> configuration, Device Tree, peripheral drivers and hardware test
> applications. The project covers GPIO, UART, I2C, SPI, PWM, ADC and
> CAN. I build the ARM kernel using a cross-compilation toolchain,
> customize Device Tree nodes and overlays, integrate drivers using
> the Linux device model, and validate each peripheral on the target
> hardware using kernel logs, Linux subsystem interfaces and
> hardware-level tests."

---

# 51. Final Checklist

Before considering the kernel build complete:

```text
[ ] Host build dependencies installed
[ ] ARM cross compiler installed
[ ] Linux kernel source downloaded
[ ] Correct kernel version selected
[ ] BeagleBone configuration selected
[ ] .config generated
[ ] Required kernel subsystems enabled
[ ] Device Tree configured
[ ] Kernel compiled successfully
[ ] Device Trees compiled successfully
[ ] Kernel modules compiled
[ ] Kernel modules installed
[ ] zImage generated
[ ] DTB generated
[ ] Kernel deployed
[ ] DTB deployed
[ ] Board booted
[ ] uname verified
[ ] Device Tree verified
[ ] dmesg checked
[ ] Driver probe verified
[ ] GPIO tested
[ ] UART tested
[ ] I2C tested
[ ] SPI tested
[ ] PWM tested
[ ] ADC tested
[ ] CAN tested
```

---

# 52. Summary

The Linux kernel build is the foundation of this BeagleBone Black
driver-development project.

The complete process is:

```text
Kernel Source
     |
     v
Board Configuration
     |
     v
Kernel Configuration
     |
     v
Device Tree
     |
     v
Cross Compilation
     |
     +------------------+
     |                  |
     v                  v
   zImage              DTB
     |                  |
     +--------+---------+
              |
              v
             U-Boot
              |
              v
         Linux Kernel
              |
              v
       Device Tree Parsing
              |
              v
        Device Creation
              |
              v
        Driver Matching
              |
              v
            probe()
              |
              v
       Hardware Initialization
              |
              v
          Linux Subsystem
              |
              v
          User-Space Test
```

For this project, the most important relationship to understand is:

```text
Device Tree
     +
Kernel Configuration
     +
Linux Driver
     +
Linux Subsystem
     +
Hardware
     =
Working Peripheral
```

The next documentation section should cover the actual **custom driver
development and implementation flow**, including `module_init()`,
`module_exit()`, `probe()`, `remove()`, `of_match_table`, `platform_driver`,
character drivers, GPIO/I2C/SPI driver examples, and how each driver is
integrated into this repository.

```
```

