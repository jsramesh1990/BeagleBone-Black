Build and test
cd beaglebone-black/user-space/spi_test
make
make read
make write
make transfer
make loopback

Or:

sudo ./spi_test /dev/spidev0.0 read
sudo ./spi_test /dev/spidev0.0 write
sudo ./spi_test /dev/spidev0.0 transfer
sudo ./spi_test /dev/spidev0.0 loopback

For the loopback test, physically connect MOSI → MISO on the SPI interface.
