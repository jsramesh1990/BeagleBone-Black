Perfect. For the I²C section, keep the same professional structure:

```text
beaglebone-black/device-tree/i2c/
├── bbb-i2c.dts
├── bbb-i2c.dtsi
└── README.md
```

Below are the complete contents for all three files.

---

# 1. `bbb-i2c.dts`

```dts
/*
 * BeagleBone Black I2C Device Tree Test
 *
 * Platform : BeagleBone Black
 * SoC      : TI AM335x
 *
 * Purpose:
 *   Enable and test I2C controllers on the BeagleBone Black.
 */

/dts-v1/;

/include/ "am335x-boneblack.dts"
/include/ "bbb-i2c.dtsi"

/ {
	model = "TI AM335x BeagleBone Black - I2C Test";
	compatible = "ti,am335x-bone-black", "ti,am335x";
};
```

---

# 2. `bbb-i2c.dtsi`

For the initial version, enable the I²C controllers without adding a
specific sensor.

```dts
/*
 * BeagleBone Black I2C configuration
 *
 * AM335x provides multiple I2C controllers.
 *
 * The exact controller availability and pinmux depend on the
 * kernel Device Tree and BeagleBone Black configuration.
 */

/*
 * I2C0
 *
 * Commonly used by the board for on-board devices.
 * Do not modify this controller blindly if your base Device Tree
 * already contains devices attached to it.
 */

&i2c0 {
	status = "okay";
};

/*
 * I2C1
 */

&i2c1 {
	status = "okay";
};

/*
 * I2C2
 */

&i2c2 {
	status = "okay";
};
```

### Important

For your GitHub project, **I²C controller enablement and I²C client-device configuration should be separated**.

For example, later you can add:

```text
i2c/
├── bbb-i2c.dts
├── bbb-i2c.dtsi
├── devices/
│   ├── mpu6050.dts
│   ├── bmp280.dts
│   └── eeprom-24c256.dts
└── README.md
```

This makes the project much stronger because you demonstrate both:

```text
I2C Controller
      ↓
I2C Bus
      ↓
I2C Client Device
      ↓
I2C Driver
```

---

# 3. `README.md`

````markdown
# BeagleBone Black I2C Device Tree

## 1. Overview

This directory contains the Device Tree configuration used for I2C
development and testing on the BeagleBone Black.

The project demonstrates:

- I2C controller configuration
- I2C bus enablement
- I2C pin multiplexing
- I2C client-device description
- Linux I2C subsystem
- I2C adapter detection
- I2C device detection
- I2C driver integration
- User-space I2C testing

The I2C section is part of the complete BeagleBone Black Linux
device-driver project.

---

# 2. Directory Structure

```text
i2c/
├── bbb-i2c.dts
├── bbb-i2c.dtsi
└── README.md
````

| File           | Purpose                        |
| -------------- | ------------------------------ |
| `bbb-i2c.dts`  | Main I2C Device Tree test file |
| `bbb-i2c.dtsi` | I2C controller configuration   |
| `README.md`    | I2C Device Tree documentation  |

---

# 3. Hardware Platform

```text
Board : BeagleBone Black
SoC   : TI AM335x
CPU   : ARM Cortex-A8
```

The AM335x SoC contains multiple I2C controllers.

The Linux kernel exposes these controllers as I2C adapters.

---

# 4. I2C Architecture

The complete I2C architecture is:

```text
                    USER SPACE
                        |
                        v
                I2C Test Application
                        |
                        v
                  /dev/i2c-X
                        |
                        v
                  Linux I2C Core
                        |
                        v
                  I2C Adapter
                        |
                        v
                  I2C Controller
                        |
                        v
                     AM335x
                        |
                 SDA          SCL
                  |            |
                  +-----+------+
                        |
                        v
                  I2C Slave Device
                        |
          +-------------+-------------+
          |             |             |
          v             v             v
       EEPROM         Sensor        RTC
```

---

# 5. I2C Communication

I2C uses two primary signals:

```text
SDA -> Serial Data
SCL -> Serial Clock
```

Typical connection:

```text
BeagleBone Black              I2C Sensor

SDA -------------------------- SDA
SCL -------------------------- SCL
GND -------------------------- GND
3.3V ------------------------- VCC
```

I2C requires appropriate pull-up resistors on SDA and SCL.

---

# 6. I2C Device Tree Flow

```text
bbb-i2c.dts
      |
      v
bbb-i2c.dtsi
      |
      v
Device Tree Compiler
      |
      v
bbb-i2c.dtb
      |
      v
Bootloader
      |
      v
Linux Kernel
      |
      v
I2C Controller
      |
      v
I2C Adapter
      |
      v
I2C Client Device
      |
      v
I2C Driver
```

---

# 7. I2C Controller Enablement

The controller configuration is located in:

```text
bbb-i2c.dtsi
```

Example:

```dts
&i2c0 {
    status = "okay";
};

&i2c1 {
    status = "okay";
};

&i2c2 {
    status = "okay";
};
```

The exact controller and pinmux configuration must match the Linux
kernel Device Tree used by the target BeagleBone Black.

---

# 8. I2C Pin Multiplexing

AM335x pins are multiplexed.

A physical pin can provide different functions:

```text
                  AM335x Pin
                       |
          +------------+------------+
          |            |            |
          v            v            v
         GPIO         I2C          UART
```

For I2C operation, the selected pins must be configured for:

```text
SDA
SCL
```

Always verify the exact BeagleBone Black header pin mapping against
the Device Tree and hardware documentation for your board.

---

# 9. Inspect Existing Device Tree

From the Linux kernel source tree:

```bash
grep -R "i2c0" arch/arm/boot/dts/
```

Search for:

```bash
grep -R "i2c1" arch/arm/boot/dts/
```

and:

```bash
grep -R "i2c2" arch/arm/boot/dts/
```

Find BeagleBone Black Device Tree files:

```bash
find arch/arm/boot/dts -iname "*boneblack*"
```

Search for I2C pinmux:

```bash
grep -R "i2c.*pins" arch/arm/boot/dts/
```

---

# 10. Build Device Tree

Check Device Tree Compiler:

```bash
dtc --version
```

Compile:

```bash
dtc -I dts -O dtb -o bbb-i2c.dtb bbb-i2c.dts
```

Output:

```text
bbb-i2c.dtb
```

---

# 11. Kernel I2C Support

Check the running kernel configuration:

```bash
grep CONFIG_I2C /boot/config-$(uname -r)
```

Important configuration categories include:

```text
CONFIG_I2C
CONFIG_I2C_CHARDEV
```

The AM335x I2C controller driver configuration depends on the
kernel version.

---

# 12. Verify I2C Device Nodes

After booting the BeagleBone Black:

```bash
ls -l /dev/i2c-*
```

Possible output:

```text
/dev/i2c-0
/dev/i2c-1
/dev/i2c-2
```

The actual adapter numbers depend on the board Device Tree and
kernel configuration.

---

# 13. Check I2C Adapters

Install I2C utilities if required:

```bash
sudo apt install i2c-tools
```

List adapters:

```bash
i2cdetect -l
```

Example:

```text
i2c-1   i2c   OMAP I2C adapter
i2c-2   i2c   OMAP I2C adapter
```

The exact adapter numbering and names depend on the kernel.

---

# 14. Scan I2C Bus

Example:

```bash
sudo i2cdetect -y 1
```

Example output:

```text
     0 1 2 3 4 5 6 7 8 9 a b c d e f
00:          -- -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --
```

If a device is connected, its address may appear in the scan.

For example:

```text
48
```

indicates a responding device at I2C address `0x48`.

---

# 15. I2C Address

Every I2C slave has an address.

Example:

```text
BeagleBone
    |
    | I2C
    |
    +---- 0x48 -> ADC
    |
    +---- 0x68 -> IMU
    |
    +---- 0x50 -> EEPROM
```

The address depends on the specific hardware.

---

# 16. I2C Client Device

A Device Tree can describe an I2C client device.

Conceptually:

```dts
&i2c2 {
    status = "okay";

    sensor@48 {
        compatible = "vendor,sensor";
        reg = <0x48>;
    };
};
```

Here:

```text
sensor@48
```

represents an I2C device at address:

```text
0x48
```

The `compatible` string must match the actual Linux driver.

Do not use a fictional `compatible` string in production hardware
configuration.

---

# 17. I2C Device Tree Architecture

```text
                   Device Tree
                       |
                       v
                   I2C Bus Node
                       |
                       v
                I2C Client Node
                       |
                       v
               compatible + reg
                       |
                       v
                 Driver Matching
                       |
                       v
                    probe()
```

---

# 18. I2C Driver Flow

The Linux I2C driver architecture is:

```text
Device Tree
     |
     v
I2C Adapter
     |
     v
I2C Client
     |
     v
Driver Matching
     |
     v
probe()
     |
     v
I2C Driver
     |
     v
I2C Transfer
     |
     v
I2C Hardware
```

---

# 19. I2C Driver Matching

The driver usually contains an I2C device ID or Device Tree match
table.

Example:

```c
static const struct of_device_id bbb_i2c_of_match[] = {
    {
        .compatible = "example,my-i2c-device",
    },
    { }
};

MODULE_DEVICE_TABLE(of, bbb_i2c_of_match);
```

The Device Tree:

```dts
compatible = "example,my-i2c-device";
```

must match the driver.

---

# 20. I2C Driver Probe

The general sequence is:

```text
Linux Boot
    |
    v
Device Tree Parsing
    |
    v
I2C Adapter Registration
    |
    v
I2C Client Creation
    |
    v
Driver Matching
    |
    v
probe()
    |
    v
Device Initialization
    |
    v
I2C Communication
```

---

# 21. I2C Read Operation

Conceptual flow:

```text
User Application
       |
       v
I2C Driver
       |
       v
i2c_transfer()
       |
       v
I2C Core
       |
       v
I2C Adapter
       |
       v
AM335x I2C Controller
       |
       v
SDA/SCL
       |
       v
I2C Slave
```

---

# 22. I2C Write Operation

```text
Application
     |
     v
I2C Driver
     |
     v
I2C Core
     |
     v
I2C Controller
     |
     v
SDA/SCL
     |
     v
I2C Slave
```

Example transaction:

```text
START
  |
  v
Slave Address
  |
  v
Write
  |
  v
Register Address
  |
  v
Data
  |
  v
STOP
```

---

# 23. User-Space I2C Testing

I2C devices can be accessed from user space through:

```text
/dev/i2c-X
```

For example:

```bash
ls -l /dev/i2c-*
```

Applications can use the Linux I2C device interface through:

```c
open()
ioctl()
read()
write()
```

The important ioctl used for selecting a slave address is:

```c
I2C_SLAVE
```

---

# 24. Example User-Space Flow

```text
open("/dev/i2c-1")
        |
        v
ioctl(I2C_SLAVE, address)
        |
        v
write()
        |
        v
read()
        |
        v
I2C Device
```

---

# 25. I2C Tools

Useful commands:

```bash
i2cdetect -l
```

Scan:

```bash
sudo i2cdetect -y 1
```

Read a register:

```bash
sudo i2cget -y 1 0x48 0x00
```

Write a register:

```bash
sudo i2cset -y 1 0x48 0x01 0x80
```

Dump registers:

```bash
sudo i2cdump -y 1 0x48
```

Use these commands only with a device whose register map you
understand. Blind writes can put hardware into an unintended state.

---

# 26. I2C Bus Speeds

Common I2C modes include:

```text
Standard Mode
100 kHz

Fast Mode
400 kHz

Fast Mode Plus
1 MHz
```

The actual supported speed depends on the controller, board design,
slave device and Device Tree configuration.

---

# 27. Pull-Up Resistors

I2C uses open-drain/open-collector style signaling.

Therefore SDA and SCL require pull-up resistors.

Conceptually:

```text
3.3V
 |
 +---- Pull-up ---- SDA
 |
 +---- Pull-up ---- SCL
```

Then:

```text
BeagleBone SDA -------- Sensor SDA
BeagleBone SCL -------- Sensor SCL
```

The appropriate pull-up value depends on bus capacitance, voltage,
speed and device requirements.

---

# 28. I2C Debugging

Check kernel messages:

```bash
dmesg | grep -i i2c
```

Check adapters:

```bash
i2cdetect -l
```

Check device nodes:

```bash
ls -l /dev/i2c-*
```

Scan bus:

```bash
sudo i2cdetect -y 1
```

Check kernel configuration:

```bash
grep CONFIG_I2C /boot/config-$(uname -r)
```

---

# 29. Device Tree Debugging

Check live Device Tree:

```bash
ls /proc/device-tree/
```

Find I2C nodes:

```bash
find /proc/device-tree -iname "*i2c*"
```

Check kernel messages:

```bash
dmesg | grep -Ei "i2c|omap"
```

---

# 30. Common Problems

## Problem 1: `/dev/i2c-*` Does Not Exist

Check:

```bash
grep CONFIG_I2C /boot/config-$(uname -r)
```

Then:

```bash
dmesg | grep -i i2c
```

Verify:

* I2C kernel support
* Controller Device Tree
* Pinmux
* Controller driver

---

## Problem 2: `i2cdetect` Shows No Device

Check:

```bash
sudo i2cdetect -y 1
```

Verify:

* Correct I2C bus
* SDA wiring
* SCL wiring
* GND
* Device power
* I2C address
* Pull-up resistors

---

## Problem 3: Wrong I2C Bus

Run:

```bash
i2cdetect -l
```

Do not assume:

```text
i2c-1 == physical I2C1
```

Linux adapter numbering can change depending on the Device Tree
and kernel configuration.

---

## Problem 4: Device Does Not Probe

Check:

```bash
dmesg | grep -Ei "i2c|driver|probe"
```

Verify:

```text
compatible
reg
I2C controller
driver
```

The `compatible` property must match the driver.

---

# 31. I2C Error Handling

Typical I2C errors include:

```text
NACK
Bus Busy
Timeout
Arbitration Lost
Transfer Failure
```

The driver should handle errors returned from I2C transfers.

Conceptually:

```c
ret = i2c_transfer(adapter, msgs, num_msgs);

if (ret < 0) {
    /* Handle transfer failure */
}
```

---

# 32. I2C Testing Matrix

| Test                 | Description                   | Status  |
| -------------------- | ----------------------------- | ------- |
| Controller Detection | Detect I2C controller         | Planned |
| Device Node          | Verify `/dev/i2c-*`           | Planned |
| Adapter Detection    | `i2cdetect -l`                | Planned |
| Bus Scan             | Detect slave                  | Planned |
| Read                 | Read device register          | Planned |
| Write                | Write device register         | Planned |
| Driver Probe         | Verify client driver          | Planned |
| Interrupt            | Test interrupt-capable device | Planned |
| Error Handling       | Test NACK/timeout             | Planned |
| Stress Test          | Continuous transfers          | Planned |
| Multi Device         | Multiple I2C slaves           | Planned |

---

# 33. Example I2C Test Setup

```text
                 BeagleBone Black
                       |
                       |
              +--------+--------+
              |                 |
             SDA               SCL
              |                 |
              |                 |
              +--------+--------+
                       |
                       v
                 I2C Bus
                       |
            +----------+----------+
            |          |          |
            v          v          v
          EEPROM      IMU       RTC
         0x50        0x68       0x51
```

---

# 34. Multi-Device I2C

One I2C bus can contain multiple slave devices.

Example:

```text
I2C Bus
  |
  +---- 0x50 EEPROM
  |
  +---- 0x68 IMU
  |
  +---- 0x76 Pressure Sensor
```

Each device must have a unique address on the bus.

---

# 35. Driver Development

The I2C Device Tree section connects to the driver:

```text
device-tree/i2c/
        |
        v
I2C Device Tree
        |
        v
drivers/04_i2c/
        |
        v
I2C Linux Driver
        |
        v
user-space/i2c_test/
        |
        v
Functional Test
```

---

# 36. Driver Directory

The main driver implementation will be:

```text
drivers/
└── 04_i2c/
```

A recommended structure is:

```text
04_i2c/
├── Makefile
├── README.md
├── bbb_i2c_driver.c
└── tests/
```

---

# 37. Driver Responsibilities

The I2C driver should demonstrate:

* I2C driver registration
* Device Tree matching
* `probe()`
* `remove()`
* I2C client handling
* Register read
* Register write
* `i2c_transfer()`
* Error handling
* Device initialization
* Kernel logging
* Module loading/unloading

---

# 38. Complete I2C Flow

```text
                 Hardware
                    |
                    v
               AM335x I2C
                    |
                    v
              Pin Multiplexing
                    |
                    v
               Device Tree
                    |
                    v
              Linux I2C Core
                    |
                    v
              I2C Adapter
                    |
                    v
              I2C Client
                    |
                    v
               I2C Driver
                    |
                    v
                 probe()
                    |
                    v
              I2C Transfers
                    |
                    v
              I2C Hardware
                    |
                    v
               I2C Slave
```

---

# 39. Development Checklist

* [ ] Identify I2C controller
* [ ] Identify SDA pin
* [ ] Identify SCL pin
* [ ] Verify pinmux
* [ ] Create `bbb-i2c.dtsi`
* [ ] Create `bbb-i2c.dts`
* [ ] Build Device Tree
* [ ] Deploy DTB
* [ ] Boot board
* [ ] Verify I2C driver
* [ ] Verify `/dev/i2c-*`
* [ ] Run `i2cdetect -l`
* [ ] Scan I2C bus
* [ ] Connect I2C sensor
* [ ] Detect slave address
* [ ] Read register
* [ ] Write register
* [ ] Develop I2C driver
* [ ] Test driver probe
* [ ] Test I2C transfers
* [ ] Test error handling
* [ ] Perform stress test
* [ ] Document results

---

# 40. Repository Integration

The I2C module is part of the complete BeagleBone Black driver project:

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
|   |
|   +-- 01_char_driver/
|   +-- 02_gpio/
|   +-- 03_interrupt/
|   +-- 04_i2c/
|   +-- ...
|
+-- user-space/
|   |
|   +-- i2c_test/
|   +-- ...
|
+-- tests/
|
+-- docs/
|
+-- scripts/
```

---

# 41. Final I2C Flow

```text
                 BEAGLEBONE BLACK
                        |
                        v
                     AM335x
                        |
                        v
                  I2C Controller
                        |
                        v
                   SDA / SCL
                        |
                        v
                    I2C Bus
                        |
          +-------------+-------------+
          |             |             |
          v             v             v
       EEPROM          IMU           RTC
        0x50          0x68          0x51
          |             |             |
          +-------------+-------------+
                        |
                        v
                   I2C Driver
                        |
                        v
                  User Space
```

---

# 42. Status

```text
Device Tree Configuration : In Development
Pinmux Configuration      : To Be Verified
I2C Controller            : In Development
I2C Adapter               : Planned
I2C Client Device         : Planned
I2C Driver                : In Development
I2C Read                  : Planned
I2C Write                 : Planned
Error Handling            : Planned
Stress Test               : Planned
Multi-Device Test         : Planned
Documentation             : In Progress
```

---

# 43. Summary

This I2C module demonstrates the complete Linux I2C development flow:

```text
AM335x I2C Controller
        ↓
Pin Multiplexing
        ↓
Device Tree
        ↓
Linux I2C Core
        ↓
I2C Adapter
        ↓
I2C Client
        ↓
I2C Driver
        ↓
I2C Transfer
        ↓
I2C Sensor / EEPROM / RTC
        ↓
User-Space Testing
```

The I2C implementation will later connect to:

```text
device-tree/i2c/
        ↓
drivers/04_i2c/
        ↓
user-space/i2c_test/
        ↓
tests/
```

````

---

## Final directory

After adding the content:

```text
beaglebone-black/device-tree/i2c/
├── bbb-i2c.dts
├── bbb-i2c.dtsi
└── README.md
````

### One important correction for the overall project

For **I²C**, I recommend that we eventually add actual client devices rather than stopping at `status = "okay"`:

```text
I2C
├── Controller enable
├── Pinmux
├── I2C bus
├── EEPROM
├── IMU
├── RTC
├── Sensor
└── Custom I2C driver
```

That will make your GitHub project demonstrate the complete **Device Tree → I²C Core → Client Driver → Hardware → User-space test** flow rather than just showing that an I²C controller was enabled.

