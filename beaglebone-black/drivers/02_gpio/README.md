Device Tree relationship

The important connection is:

Device Tree
     │
     ├── led-gpios
     │       │
     │       ▼
     │   gpio_led.c
     │       │
     │       ▼
     │   /dev/bbb_gpio_led
     │
     └── button-gpios
             │
             ▼
       gpio_button.c
             │
             ▼
       /dev/bbb_gpio_button

So the strings in the headers:

#define GPIO_LED_CONSUMER    "led"
#define GPIO_BUTTON_CONSUMER "button"

correspond to the Device Tree properties:

led-gpios = <...>;
button-gpios = <...>;

This keeps your GPIO driver implementation clean and makes the Device Tree → GPIO framework → driver → /dev → user-space application flow easy to understand.
