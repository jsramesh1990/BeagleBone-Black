Build
cd beaglebone-black/user-space/i2c_test
make
Test
make scan
make read
make write
make loopback

Or manually:

sudo ./i2c_test /dev/i2c-2 0x50 scan
sudo ./i2c_test /dev/i2c-2 0x50 read
sudo ./i2c_test /dev/i2c-2 0x50 write
sudo ./i2c_test /dev/i2c-2 0x50 loopback

Note: 0x50 is only an example slave address. Use the actual address of the I²C device connected to your BeagleBone Black.
