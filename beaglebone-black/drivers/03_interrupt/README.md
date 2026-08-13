Build and load
cd beaglebone-black/drivers/03_interrupt

make
sudo insmod gpio_irq.ko

Check:

make status
make irq
make logs

Remove:

make unload

The complete interrupt path is:

Device Tree
    │
    │ irq-gpios
    ▼
GPIO Descriptor
    │
    ▼
gpiod_to_irq()
    │
    ▼
request_irq()
    │
    ▼
GPIO Hardware Event
    │
    ▼
gpio_irq_handler()
    │
    ├── increment IRQ counter
    └── wake waiting process
              │
              ▼
       /dev/bbb_gpio_irq
              │
              ▼
       User-space application

One important point: the exact irq-gpios GPIO and its interrupt trigger must match your BeagleBone Black Device Tree/pinmux configuration.
