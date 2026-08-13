Make it executable:

chmod +x beaglebone-black/scripts/build.sh

Then from the project root:

cd beaglebone-black
./scripts/build.sh help

Build everything:

./scripts/build.sh all

Build only the kernel:

./scripts/build.sh kernel

Build Device Tree:

./scripts/build.sh dtb

Build modules:

./scripts/build.sh modules

Clean:

./scripts/build.sh clean
One correction for your repository

Your current project has:

beaglebone-black/
├── kernel/
│   ├── config/
│   └── patches/
├── device-tree/
├── hardware/
└── scripts/
    └── build.sh

The script above expects the actual Linux kernel source at:

beaglebone-black/kernel/linux/

So eventually your structure should contain:

kernel/
├── linux/                    # Linux kernel source
├── config/
│   ├── bbb_defconfig
│   └── bbb_driver_defconfig
└── patches/
    ├── 0001-...
    ├── 0002-...
    └── ...

This gives you a clean build flow:

                build.sh
                   |
        +----------+----------+
        |          |          |
        v          v          v
      Config      Kernel      DTB
        |          |          |
        +----------+----------+
                   |
                   v
               Modules
                   |
                   v
                output/
             ├── zImage
             ├── dtbs/
             └── modules/
                   |
                   v
             BeagleBone Black
                   |
                   v
        GPIO / I2C / SPI / UART
             PWM / CAN / ADC

========================================
[200~Make it executable:

chmod +x beaglebone-black/scripts/collect_logs.sh

Run it on the BeagleBone Black:

./scripts/collect_logs.sh

It will create:

beaglebone-black/
└── logs/
    ├── bbb_20260813_103000/
    │   ├── README.txt
    │   ├── dmesg.log
    │   ├── dmesg_errors.log
    │   ├── gpio.log
    │   ├── i2c_bus.log
    │   ├── spi_devices.log
    │   ├── serial_devices.log
    │   ├── pwm.log
    │   ├── can_interfaces.log
    │   ├── iio_devices.log
    │   ├── device_tree.log
    │   ├── loaded_modules.log
    │   ├── interrupts.log
    │   └── ...
    │
    └── bbb_logs_20260813_103000.tar.gz
Main debugging flow
collect_logs.sh
       |
       +---- dmesg
       |
       +---- Device Tree
       |
       +---- GPIO
       |
       +---- I2C
       |
       +---- SPI
       |
       +---- UART
       |
       +---- PWM
       |
       +---- CAN
       |
       +---- ADC / IIO
       |
       +---- Kernel Modules
       |
       +---- Interrupts
       |
       v
   logs/*.log
       |
       v
 Debug / Analyze / Share

This makes collect_logs.sh useful as the single diagnostic script for your complete BeagleBone Black driver project.
=====================================================================
Make it executable:

chmod +x beaglebone-black/scripts/install.sh
Usage

Build first:

./scripts/build.sh all

Then install everything:

sudo ./scripts/install.sh

Or:

sudo ./scripts/install.sh all

Individual installation:

sudo ./scripts/install.sh kernel
sudo ./scripts/install.sh dtb
sudo ./scripts/install.sh modules
sudo ./scripts/install.sh drivers

Check installed modules:

ls /lib/modules/$(uname -r)/extra/bbb/

Check Device Tree:

ls /boot/dtbs/

Check kernel:

ls -lh /boot/zImage-bbb
Project deployment flow
                  build.sh
                     |
                     v
              +--------------+
              | Linux Kernel  |
              +--------------+
                     |
              +------+------+
              |             |
              v             v
            zImage         DTB
              |             |
              +------+------+
                     |
                     v
                install.sh
                     |
       +-------------+-------------+
       |             |             |
       v             v             v
     /boot        /boot/dtbs   /lib/modules
       |             |             |
       +-------------+-------------+
                     |
                     v
              BeagleBone Black
                     |
                     v
       GPIO / I2C / SPI / UART
          PWM / CAN / ADC

Important: this script assumes output/ is produced by your build.sh. For a production BBB deployment, you should normally install into the board's boot/root filesystem from a controlled target filesystem rather than blindly overwriting the running board's /boot; especially keep a known-good kernel and DTB available for recovery.
-============================================================================
Make it executable:

chmod +x beaglebone-black/scripts/test_all.sh

Run the complete test:

sudo ./scripts/test_all.sh

Or test individual peripherals:

sudo ./scripts/test_all.sh gpio
sudo ./scripts/test_all.sh adc
sudo ./scripts/test_all.sh i2c
sudo ./scripts/test_all.sh spi
sudo ./scripts/test_all.sh uart
sudo ./scripts/test_all.sh pwm
sudo ./scripts/test_all.sh can

The result is stored under:

beaglebone-black/
└── logs/
    └── test_YYYYMMDD_HHMMSS.log
Overall project test flow
                    test_all.sh
                         |
          +--------------+--------------+
          |              |              |
          v              v              v
     Device Tree     Kernel Modules   Interrupts
          |
          +----------------------------------+
          |        Peripheral Tests          |
          +----------------------------------+
          |                                  |
          v                                  v
        GPIO                              ADC / IIO
          |                                  |
          v                                  v
         I2C                                SPI
          |                                  |
          v                                  v
        UART                                PWM
          |                                  |
          +---------------+------------------+
                          |
                          v
                         CAN
                          |
                          v
                    TEST SUMMARY
                    PASS / FAIL / SKIP

Note: the script intentionally performs non-destructive discovery/read tests by default. SPI loopback, UART TX/RX loopback, CAN TX/RX, GPIO output toggling, and PWM waveform verification require connected test hardware, so those should be added as explicit hardware-test modes rather than automatically driving pins.

============================================================================================================
Make it executable:

chmod +x beaglebone-black/scripts/unload.sh
Usage

Check currently loaded project drivers:

sudo ./scripts/unload.sh list

Unload one driver:

sudo ./scripts/unload.sh gpio

Unload all project drivers:

sudo ./scripts/unload.sh all

Verify:

lsmod | grep bbb
Driver lifecycle
                  Driver .ko
                      |
                      v
                 insmod/modprobe
                      |
                      v
                Device Tree
                      |
                      v
                   probe()
                      |
                      v
             Hardware / Framework
                      |
                      |
                unload.sh
                      |
                      v
                  remove()
                      |
                      v
                modprobe -r
                      |
                      v
                Driver unloaded

Important: don't use this script to remove Linux's standard AM335x subsystem drivers (8250, ti_dcan, ti_am335x_adc, PWM provider, etc.) just to test your project. Your bbb_* modules should be the modules you own; the standard kernel drivers should remain loaded unless you're deliberately doing low-level driver replacement testing.
