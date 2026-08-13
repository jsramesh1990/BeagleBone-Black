Final structure
beaglebone-black/
└── drivers/
    └── 04_i2c/
        ├── i2c_eeprom.c
        ├── i2c_eeprom.h
        ├── i2c_sensor.c
        ├── i2c_sensor.h
        └── Makefile
Build
cd beaglebone-black/drivers/04_i2c
make

Check the generated modules:

ls -l *.ko

Expected:

i2c_eeprom.ko
i2c_sensor.ko

Note: 0x50 and 0x48 are example addresses. The actual EEPROM/sensor address and register map must match the hardware and Device Tree configuration on your BeagleBone Black.
