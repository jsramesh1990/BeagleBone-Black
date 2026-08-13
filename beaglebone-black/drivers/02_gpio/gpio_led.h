/*
 * BeagleBone Black - GPIO LED Driver Header
 *
 * File:
 *     gpio_led.h
 */

#ifndef GPIO_LED_H
#define GPIO_LED_H


/* ------------------------------------------------------------------------- */
/* Driver Information                                                        */
/* ------------------------------------------------------------------------- */

#define GPIO_LED_DRIVER_NAME          "bbb_gpio_led"

#define GPIO_LED_CLASS_NAME           "bbb_gpio_led_class"

#define GPIO_LED_DEVICE_NAME          "bbb_gpio_led"


/* ------------------------------------------------------------------------- */
/* Device Tree GPIO Consumer                                                */
/* ------------------------------------------------------------------------- */

/*
 * Expected Device Tree property:
 *
 *     led-gpios = <...>;
 *
 * gpiod_get(NULL, "led", GPIOD_OUT_LOW)
 * searches for the "led-gpios" property.
 */
#define GPIO_LED_CONSUMER             "led"


/* ------------------------------------------------------------------------- */
/* LED Configuration                                                         */
/* ------------------------------------------------------------------------- */

#define GPIO_LED_ON                   1

#define GPIO_LED_OFF                  0


/* ------------------------------------------------------------------------- */
/* Driver Return Values                                                      */
/* ------------------------------------------------------------------------- */

#define GPIO_LED_SUCCESS              0

#define GPIO_LED_ERROR                -1


#endif /* GPIO_LED_H */
