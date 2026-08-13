Run
cd beaglebone-black/tools
chmod +x device_scan.sh
sudo ./device_scan.sh

The script gives you a single BBB hardware/peripheral inventory covering:

Device Tree
   ↓
GPIO
   ↓
I2C
   ↓
SPI
   ↓
UART
   ↓
PWM
   ↓
CAN
   ↓
Ethernet
   ↓
USB / PCIe
   ↓
MMC / eMMC / SD
   ↓
Kernel Drivers
   ↓
Interrupts / DMA
   ↓
Kernel Error Logs

It also saves the complete scan under:

beaglebone-black/logs/device_scan_<timestamp>.log

=======================================================================================================


Run
cd beaglebone-black/tools
chmod +x driver_status.sh
sudo ./driver_status.sh

It checks the complete driver stack:

Device Tree
    ↓
GPIO Driver
    ↓
I2C Driver
    ↓
SPI Driver
    ↓
UART Driver
    ↓
PWM Driver
    ↓
CAN Driver
    ↓
Ethernet Driver
    ↓
USB Driver
    ↓
MMC/eMMC Driver
    ↓
Kernel Modules
    ↓
Platform Drivers
    ↓
Driver Bindings
    ↓
Interrupts
    ↓
Kernel Driver Errors

The output is also saved under:

beaglebone-black/logs/driver_status_<timestamp>.log

================================================================================================
Run
cd beaglebone-black/tools
chmod +x gpio_monitor.sh
sudo ./gpio_monitor.sh /dev/gpiochip0 20

For faster monitoring:

sudo INTERVAL=0.1 ./gpio_monitor.sh /dev/gpiochip0 20
What it monitors
GPIO Controller
      │
      ▼
/dev/gpiochipX
      │
      ▼
GPIO Line
      │
      ├── Current Value
      ├── HIGH / LOW
      ├── State Changes
      ├── Read Errors
      └── Runtime Statistics

The monitoring log is saved to:

beaglebone-black/logs/gpio_monitor_<timestamp>.log

Important: On modern Linux/BeagleBone Black systems, use the libgpiod interface (gpioget, gpioinfo, gpiodetect) rather than the older /sys/class/gpio interface.

===================================================================================

Run
cd beaglebone-black/tools
chmod +x irq_monitor.sh
sudo ./irq_monitor.sh

For a 2-minute monitor with 1-second sampling:

sudo ./irq_monitor.sh 1 120

The script monitors the IRQ flow:

Peripheral
    │
    ▼
Hardware Event
    │
    ▼
IRQ
    │
    ▼
Linux Interrupt Controller
    │
    ▼
ISR / Interrupt Handler
    │
    ▼
Driver

It specifically tracks GPIO, I2C, SPI, UART, PWM, CAN, Ethernet, USB, MMC and DMA interrupt activity and saves the results under:

beaglebone-black/logs/irq_monitor_<timestamp>.log







