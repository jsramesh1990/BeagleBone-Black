Build
cd beaglebone-black/user-space/adc_test
make

This produces:

adc_test
adc_test.o
Run
sudo ./adc_test

Or specify channel and samples:

sudo ./adc_test 0 20
Clean
make clean
Install
sudo make install

The executable will be installed as:

/usr/local/bin/adc_test

So your final directory becomes:

adc_test/
├── adc_test.c
├── adc_test.h
├── Makefile
└── README.md
