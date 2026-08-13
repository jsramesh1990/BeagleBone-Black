Yes. This `overlays` directory should demonstrate **runtime Device Tree modification** for your BeagleBone Black driver project.

One correction first: you have a typo:

```text
bbb-spi-overlay.dtss
```

Rename it to:

```text
bbb-spi-overlay.dts
```

Run:

```bash
cd ~/beaglebone-black/device-tree/overlays

mv bbb-spi-overlay.dtss bbb-spi-overlay.dts
```

Your final structure should be:

```text
beaglebone-black/device-tree/overlays/
├── bbb-gpio-overlay.dts
├── bbb-i2c-overlay.dts
├── bbb-spi-overlay.dts
├── bbb-uart-overlay.dts
└── README.md
```

Below are the complete files.

---

# 1. `bbb-gpio-overlay.dts`

```dts
/*
 * BeagleBone Black GPIO Device Tree Overlay
 *
 * Purpose:
 *   Configure a GPIO pin for GPIO testing.
 *
 * Platform:
 *   BeagleBone Black
 *   TI AM335x
 */

/dts-v1/;

/plugin/;

/ {
	compatible = "ti,am335x-bone-black", "ti,am335x";

	/*
	 * GPIO pin configuration.
	 *
	 * The exact GPIO and header pin must be verified against
	 * the target BeagleBone Black hardware configuration.
	 */

	fragment@0 {
		target = <&am33xx_pinmux>;

		__overlay__ {

			bbb_gpio_pins: pinmux_bbb_gpio_pins {
				pinctrl-single,pins = <
					/*
					 * GPIO pin configuration goes here.
					 *
					 * Replace this value with the correct
					 * AM335x pinmux value for the selected
					 * physical header pin.
					 */
				>;
			};
		};
	};

	fragment@1 {
		target = <&gpio1>;

		__overlay__ {
			status = "okay";
		};
	};
};
```

### Important

I intentionally did **not** put a fake pinmux hexadecimal value here.

For AM335x, the pinmux value must correspond to the **exact physical header pin and desired mode**. We'll determine the correct values when we build the hardware-specific GPIO section.

---

# 2. `bbb-i2c-overlay.dts`

```dts
/*
 * BeagleBone Black I2C Device Tree Overlay
 *
 * Purpose:
 *   Enable an I2C controller and demonstrate an I2C client device.
 *
 * Platform:
 *   BeagleBone Black
 *   TI AM335x
 */

/dts-v1/;

/plugin/;

/ {
	compatible = "ti,am335x-bone-black", "ti,am335x";

	/*
	 * I2C controller
	 */
	fragment@0 {
		target = <&i2c2>;

		__overlay__ {
			status = "okay";
		};
	};

	/*
	 * I2C client-device example.
	 *
	 * Do NOT use this node unless the actual hardware connected
	 * to the bus matches the compatible string and address.
	 */
};
```

This demonstrates:

```text
Overlay
   |
   v
&i2c2
   |
   v
status = "okay"
   |
   v
I2C Controller
```

Later, when you connect a real device such as an EEPROM, RTC, IMU, or sensor, we can add the actual child node.

For example:

```dts
fragment@1 {
	target = <&i2c2>;

	__overlay__ {
		status = "okay";

		sensor@48 {
			compatible = "vendor,device";
			reg = <0x48>;
		};
	};
};
```

But the `compatible` must correspond to a real driver.

---

# 3. `bbb-spi-overlay.dts`

```dts
/*
 * BeagleBone Black SPI Device Tree Overlay
 *
 * Purpose:
 *   Enable SPI controller for SPI device testing.
 *
 * Platform:
 *   BeagleBone Black
 *   TI AM335x
 */

/dts-v1/;

/plugin/;

/ {
	compatible = "ti,am335x-bone-black", "ti,am335x";

	/*
	 * Enable SPI0
	 */
	fragment@0 {
		target = <&spi0>;

		__overlay__ {
			status = "okay";
		};
	};
};
```

The flow is:

```text
SPI Overlay
     |
     v
&spi0
     |
     v
status = "okay"
     |
     v
SPI Controller
```

Later we can add an actual SPI slave device:

```text
SPI
 |
 +---- SPI Flash
 |
 +---- ADC
 |
 +---- Display
 |
 +---- IMU
```

---

# 4. `bbb-uart-overlay.dts`

```dts
/*
 * BeagleBone Black UART Device Tree Overlay
 *
 * Purpose:
 *   Enable a UART controller for serial communication testing.
 *
 * Platform:
 *   BeagleBone Black
 *   TI AM335x
 */

/dts-v1/;

/plugin/;

/ {
	compatible = "ti,am335x-bone-black", "ti,am335x";

	/*
	 * Enable UART1
	 */
	fragment@0 {
		target = <&uart1>;

		__overlay__ {
			status = "okay";
		};
	};
};
```

The flow:

```text
UART Overlay
     |
     v
&uart1
     |
     v
status = "okay"
     |
     v
UART Controller
     |
     v
/dev/ttyO1
```

**Note:** the exact `/dev/tty*` name depends on the kernel/driver configuration. Don't hard-code `ttyO1` as universally correct.

---

# 5. `README.md`

This is the important documentation file for your GitHub project.

````markdown
# BeagleBone Black Device Tree Overlays

## 1. Overview

This directory contains Device Tree Overlay examples for the
BeagleBone Black.

Device Tree Overlays allow hardware configuration to be added or
modified without maintaining a completely separate base Device Tree.

The overlays in this directory demonstrate:

- GPIO configuration
- I2C controller enablement
- SPI controller enablement
- UART controller enablement
- Pin multiplexing
- Runtime hardware description
- Device Tree compilation
- Overlay deployment
- Overlay verification
- Linux driver binding

---

# 2. Directory Structure

```text
overlays/
├── bbb-gpio-overlay.dts
├── bbb-i2c-overlay.dts
├── bbb-spi-overlay.dts
├── bbb-uart-overlay.dts
└── README.md
````

| File                   | Purpose                    |
| ---------------------- | -------------------------- |
| `bbb-gpio-overlay.dts` | GPIO configuration overlay |
| `bbb-i2c-overlay.dts`  | I2C controller overlay     |
| `bbb-spi-overlay.dts`  | SPI controller overlay     |
| `bbb-uart-overlay.dts` | UART controller overlay    |
| `README.md`            | Overlay documentation      |

---

# 3. What Is a Device Tree Overlay?

A Device Tree Overlay is a small Device Tree fragment that modifies
an existing Device Tree.

Instead of creating an entire Device Tree:

```text
Base Device Tree
       +
Overlay
       |
       v
Modified Device Tree
```

For example:

```text
Base DT
  |
  +---- GPIO
  +---- I2C
  +---- SPI
  +---- UART
  |
  v
Overlay
  |
  v
Enable / Modify Hardware
```

---

# 4. Device Tree vs Device Tree Overlay

## Normal Device Tree

```text
Board
 |
 v
Complete Device Tree
 |
 v
Kernel
```

## Device Tree Overlay

```text
Board
 |
 v
Base Device Tree
 |
 +------ Overlay
 |
 v
Modified Device Tree
 |
 v
Kernel
```

The overlay approach is useful when hardware configurations need to
be changed or added without replacing the complete board Device Tree.

---

# 5. Overlay Architecture

```text
                    BeagleBone Black
                           |
                           v
                    Base Device Tree
                           |
                           v
                    Device Tree Overlay
                           |
             +-------------+-------------+
             |             |             |
             v             v             v
           GPIO           I2C           SPI
             |             |             |
             +-------------+-------------+
                           |
                           v
                     Linux Kernel
                           |
                           v
                    Device Drivers
```

---

# 6. Overlay Files

The project contains four overlays:

```text
GPIO
I2C
SPI
UART
```

Each overlay demonstrates a different Linux peripheral.

---

# 7. GPIO Overlay

File:

```text
bbb-gpio-overlay.dts
```

Purpose:

```text
Configure GPIO hardware
```

Basic flow:

```text
Overlay
   |
   v
GPIO Controller
   |
   v
GPIO Pin
   |
   v
GPIO Driver
   |
   v
LED / Button
```

The GPIO overlay uses:

```dts
/plugin/;
```

which indicates that the file is a Device Tree Plugin/Overlay.

---

# 8. I2C Overlay

File:

```text
bbb-i2c-overlay.dts
```

Purpose:

```text
Enable an I2C controller.
```

Example:

```dts
fragment@0 {
    target = <&i2c2>;

    __overlay__ {
        status = "okay";
    };
};
```

Flow:

```text
Overlay
   |
   v
I2C Controller
   |
   v
I2C Adapter
   |
   v
I2C Bus
   |
   +---- Sensor
   +---- EEPROM
   +---- RTC
```

---

# 9. SPI Overlay

File:

```text
bbb-spi-overlay.dts
```

Purpose:

```text
Enable SPI controller.
```

Example:

```dts
fragment@0 {
    target = <&spi0>;

    __overlay__ {
        status = "okay";
    };
};
```

Flow:

```text
Overlay
   |
   v
SPI Controller
   |
   v
SPI Bus
   |
   +---- SPI Flash
   +---- ADC
   +---- Display
```

---

# 10. UART Overlay

File:

```text
bbb-uart-overlay.dts
```

Purpose:

```text
Enable UART controller.
```

Example:

```dts
fragment@0 {
    target = <&uart1>;

    __overlay__ {
        status = "okay";
    };
};
```

Flow:

```text
Overlay
   |
   v
UART Controller
   |
   v
UART Driver
   |
   v
/dev/tty*
   |
   v
User Application
```

---

# 11. Overlay Structure

A typical overlay contains:

```dts
/dts-v1/;

/plugin/;

/ {
    compatible = "...";

    fragment@0 {
        target = <&device>;

        __overlay__ {
            status = "okay";
        };
    };
};
```

The important sections are:

```text
/dts-v1/
    |
    v
/plugin/
    |
    v
compatible
    |
    v
fragment
    |
    v
target
    |
    v
__overlay__
```

---

# 12. Fragment

A fragment identifies which part of the existing Device Tree should
be modified.

Example:

```dts
fragment@0 {
    target = <&i2c2>;

    __overlay__ {
        status = "okay";
    };
};
```

Here:

```text
fragment@0
     |
     v
target = &i2c2
     |
     v
Modify I2C2
```

---

# 13. Target

The `target` identifies an existing Device Tree node.

Example:

```dts
target = <&spi0>;
```

means:

```text
Find SPI0 in the base Device Tree
```

Then the overlay modifies it.

---

# 14. `__overlay__`

The modifications are placed inside:

```dts
__overlay__ {
    ...
};
```

Example:

```dts
__overlay__ {
    status = "okay";
};
```

This changes the target node's status.

---

# 15. Overlay Compilation

Install Device Tree Compiler:

```bash
sudo apt install device-tree-compiler
```

Verify:

```bash
dtc --version
```

Compile GPIO overlay:

```bash
dtc -@ -I dts -O dtb \
    -o bbb-gpio-overlay.dtbo \
    bbb-gpio-overlay.dts
```

Compile I2C:

```bash
dtc -@ -I dts -O dtb \
    -o bbb-i2c-overlay.dtbo \
    bbb-i2c-overlay.dts
```

Compile SPI:

```bash
dtc -@ -I dts -O dtb \
    -o bbb-spi-overlay.dtbo \
    bbb-spi-overlay.dts
```

Compile UART:

```bash
dtc -@ -I dts -O dtb \
    -o bbb-uart-overlay.dtbo \
    bbb-uart-overlay.dts
```

The `-@` option is important for overlays because it preserves
symbol information required for resolving overlay targets.

---

# 16. Output Files

After compilation:

```text
overlays/
├── bbb-gpio-overlay.dts
├── bbb-gpio-overlay.dtbo
├── bbb-i2c-overlay.dts
├── bbb-i2c-overlay.dtbo
├── bbb-spi-overlay.dts
├── bbb-spi-overlay.dtbo
├── bbb-uart-overlay.dts
├── bbb-uart-overlay.dtbo
└── README.md
```

Usually, generated `.dtbo` files should be treated as build artifacts
rather than hand-maintained source files.

---

# 17. Validate Overlay

Use:

```bash
dtc -I dtb -O dts -o /tmp/overlay.dts bbb-i2c-overlay.dtbo
```

Then inspect:

```bash
cat /tmp/overlay.dts
```

You can also check the compiled output with:

```bash
fdtdump bbb-i2c-overlay.dtbo
```

if `fdtdump` is available.

---

# 18. Deploy Overlay

Copy the compiled overlay to the target BeagleBone Black.

Example:

```bash
scp bbb-i2c-overlay.dtbo debian@beaglebone:/tmp/
```

The exact deployment directory depends on the bootloader and Linux
distribution configuration.

Common locations include:

```text
/boot/firmware/overlays/
/boot/dtbs/overlays/
```

Always inspect the target system before copying:

```bash
ls /boot
```

and:

```bash
find /boot -type d -iname "*overlay*"
```

---

# 19. Bootloader Integration

On BeagleBone Black systems using U-Boot, overlays may be loaded by
the boot configuration.

The exact method depends on the U-Boot version and Linux distribution.

Conceptually:

```text
U-Boot
   |
   +---- Base DTB
   |
   +---- Overlay DTBO
   |
   v
Final Device Tree
   |
   v
Linux Kernel
```

---

# 20. Overlay Loading Flow

```text
Power ON
   |
   v
Boot ROM
   |
   v
SPL
   |
   v
U-Boot
   |
   +---- Load Base DTB
   |
   +---- Load Overlay
   |
   v
Apply Overlay
   |
   v
Final Device Tree
   |
   v
Linux Kernel
   |
   v
Drivers Probe
```

---

# 21. Verify Device Tree

After boot:

```bash
ls /proc/device-tree
```

Search for I2C:

```bash
find /proc/device-tree -iname "*i2c*"
```

Search for SPI:

```bash
find /proc/device-tree -iname "*spi*"
```

Search for UART:

```bash
find /proc/device-tree -iname "*uart*"
```

Search for GPIO:

```bash
find /proc/device-tree -iname "*gpio*"
```

---

# 22. Verify I2C

Check:

```bash
ls /dev/i2c-*
```

Then:

```bash
i2cdetect -l
```

Scan:

```bash
sudo i2cdetect -y 1
```

The bus number depends on the actual kernel configuration.

---

# 23. Verify SPI

Check SPI devices:

```bash
ls -l /dev/spidev*
```

Example:

```text
/dev/spidev0.0
```

The exact device node depends on the SPI controller, chip-select
configuration and kernel configuration.

---

# 24. Verify UART

Check serial devices:

```bash
ls -l /dev/tty*
```

You can filter:

```bash
ls -l /dev/ttyO*
```

or:

```bash
ls -l /dev/ttyS*
```

depending on the kernel/device naming.

---

# 25. Verify GPIO

Check GPIO chips:

```bash
ls -l /dev/gpiochip*
```

If `libgpiod` tools are installed:

```bash
gpioinfo
```

---

# 26. Kernel Logs

Check overlay/device initialization:

```bash
dmesg | grep -Ei "overlay|gpio|i2c|spi|uart"
```

Live monitoring:

```bash
dmesg -w
```

---

# 27. Device Tree Debugging

The live Device Tree is available through:

```text
/proc/device-tree/
```

Example:

```bash
find /proc/device-tree -type d | grep i2c
```

Inspect properties:

```bash
hexdump -C /proc/device-tree/<node>/status
```

---

# 28. Overlay and Driver Relationship

An overlay does not itself implement a Linux driver.

The relationship is:

```text
Device Tree Overlay
        |
        v
Hardware Description
        |
        v
Linux Device
        |
        v
Driver Matching
        |
        v
Linux Driver
        |
        v
Hardware
```

This is a very important concept for the project.

---

# 29. Example I2C Driver Flow

```text
bbb-i2c-overlay.dts
        |
        v
&i2c2 enabled
        |
        v
I2C Client Node
        |
        v
Linux I2C Core
        |
        v
compatible
        |
        v
I2C Driver
        |
        v
probe()
        |
        v
Device Initialization
        |
        v
I2C Transfers
```

---

# 30. Example SPI Driver Flow

```text
bbb-spi-overlay.dts
        |
        v
SPI Controller
        |
        v
SPI Device
        |
        v
SPI Core
        |
        v
SPI Driver
        |
        v
probe()
        |
        v
SPI Transfer
        |
        v
Hardware
```

---

# 31. Example UART Driver Flow

```text
bbb-uart-overlay.dts
        |
        v
UART Controller
        |
        v
UART Driver
        |
        v
TTY Layer
        |
        v
/dev/tty*
        |
        v
User Application
```

---

# 32. Example GPIO Driver Flow

```text
bbb-gpio-overlay.dts
        |
        v
GPIO Controller
        |
        v
GPIO Subsystem
        |
        v
GPIO Driver
        |
        v
LED / Button
```

---

# 33. Overlay Testing Matrix

| Overlay | Peripheral | Verification   | Status  |
| ------- | ---------- | -------------- | ------- |
| GPIO    | GPIO       | `gpioinfo`     | Planned |
| I2C     | I2C        | `i2cdetect`    | Planned |
| SPI     | SPI        | `/dev/spidev*` | Planned |
| UART    | UART       | `/dev/tty*`    | Planned |

---

# 34. Development Checklist

## GPIO

* [ ] Create GPIO overlay
* [ ] Configure pinmux
* [ ] Compile `.dtbo`
* [ ] Deploy overlay
* [ ] Verify GPIO controller
* [ ] Test GPIO output
* [ ] Test GPIO input
* [ ] Test GPIO interrupt

## I2C

* [ ] Create I2C overlay
* [ ] Enable controller
* [ ] Configure pinmux
* [ ] Compile `.dtbo`
* [ ] Deploy overlay
* [ ] Verify `/dev/i2c-*`
* [ ] Run `i2cdetect`
* [ ] Connect I2C device
* [ ] Test I2C driver

## SPI

* [ ] Create SPI overlay
* [ ] Enable SPI controller
* [ ] Configure pinmux
* [ ] Configure chip select
* [ ] Compile `.dtbo`
* [ ] Deploy overlay
* [ ] Verify `/dev/spidev*`
* [ ] Test SPI transfer

## UART

* [ ] Create UART overlay
* [ ] Enable UART controller
* [ ] Configure pinmux
* [ ] Compile `.dtbo`
* [ ] Deploy overlay
* [ ] Verify `/dev/tty*`
* [ ] Test TX
* [ ] Test RX

---

# 35. Common Problems

## Overlay Does Not Compile

Check:

```bash
dtc -@ -I dts -O dtb \
    -o test.dtbo \
    bbb-i2c-overlay.dts
```

Look for:

```text
syntax error
label not found
phandle error
```

---

## Target Label Does Not Exist

Example:

```dts
target = <&i2c2>;
```

The base Device Tree must provide the corresponding label/symbol.

Inspect the kernel Device Tree source and generated DTB before assuming
a label exists.

---

## Peripheral Does Not Appear

Check:

```bash
dmesg | grep -Ei "i2c|spi|uart|gpio"
```

Then verify:

```text
Overlay
   |
   v
Target node
   |
   v
Pinmux
   |
   v
Controller
   |
   v
Driver
```

---

# 36. Important Hardware Rule

A Device Tree Overlay only describes hardware.

It does not physically configure:

```text
Voltage
Pull-up resistor
External wiring
Signal integrity
```

For example, enabling I2C in Device Tree does not replace the required
SDA/SCL electrical connections and pull-ups.

---

# 37. Repository Integration

The overlay system is part of the complete BeagleBone Black project:

```text
beaglebone-black/
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
|
+-- user-space/
|
+-- tests/
|
+-- docs/
|
+-- scripts/
```

---

# 38. Complete Overlay Architecture

```text
                         BeagleBone Black
                                |
                                v
                          Base Device Tree
                                |
                                v
                       Device Tree Overlay
                                |
       +----------------+-------+-------+----------------+
       |                |               |                |
       v                v               v                v
      GPIO             I2C             SPI              UART
       |                |               |                |
       v                v               v                v
   GPIO Core        I2C Core         SPI Core          TTY Core
       |                |               |                |
       v                v               v                v
   GPIO Driver      I2C Driver      SPI Driver       UART Driver
       |                |               |                |
       v                v               v                v
     LED/SW          Sensor/EEPROM    Flash/ADC        Serial
```

---

# 39. Final Development Flow

```text
             DEVICE TREE SOURCE
                     |
                     v
              *.dts / *.dtsi
                     |
                     v
             Device Tree Compiler
                     |
                     v
                  *.dtbo
                     |
                     v
                  U-Boot
                     |
                     v
             Overlay Application
                     |
                     v
              Final Device Tree
                     |
                     v
                Linux Kernel
                     |
                     v
              Driver Matching
                     |
                     v
                  Driver
                     |
                     v
                 Hardware
```

---

# 40. Status

```text
GPIO Overlay       : In Development
I2C Overlay        : In Development
SPI Overlay        : In Development
UART Overlay       : In Development

Pinmux             : To Be Verified
Overlay Compilation: Planned
Overlay Deployment : Planned
Runtime Testing    : Planned
Driver Integration : Planned
Documentation      : In Progress
```

---

# 41. Summary

This directory demonstrates how Device Tree Overlays can be used to
configure BeagleBone Black peripherals.

The complete flow is:

```text
DTS
 ↓
DT Compiler
 ↓
DTBO
 ↓
U-Boot
 ↓
Overlay Application
 ↓
Linux Device Tree
 ↓
Linux Subsystem
 ↓
Driver
 ↓
Hardware
```

The project covers:

* GPIO
* I2C
* SPI
* UART
* Pin multiplexing
* Device Tree fragments
* Overlay compilation
* Overlay deployment
* Driver matching
* Peripheral testing
* Linux kernel integration

````

---

## Final result

Your directory should now look like:

```text
beaglebone-black/device-tree/overlays/
│
├── bbb-gpio-overlay.dts
├── bbb-i2c-overlay.dts
├── bbb-spi-overlay.dts
├── bbb-uart-overlay.dts
└── README.md
````

And compile all four with:

```bash
cd ~/beaglebone-black/device-tree/overlays

for file in *.dts; do
    dtc -@ -I dts -O dtb \
        -o "${file%.dts}.dtbo" \
        "$file"
done
```

Then:

```bash
ls -lh
```

You should see:

```text
bbb-gpio-overlay.dts
bbb-gpio-overlay.dtbo

bbb-i2c-overlay.dts
bbb-i2c-overlay.dtbo

bbb-spi-overlay.dts
bbb-spi-overlay.dtbo

bbb-uart-overlay.dts
bbb-uart-overlay.dtbo

README.md
```

**One project-design point:** don't make the overlays just four `status = "okay"` examples. For the final GitHub project, we should progressively make each one a **real hardware demonstration**—GPIO LED/button, I²C sensor/EEPROM, SPI device, and UART peripheral—then connect each to its corresponding Linux driver. That will make this a much stronger **"BeagleBone Black — Complete Linux Device Driver Development"** portfolio project.

