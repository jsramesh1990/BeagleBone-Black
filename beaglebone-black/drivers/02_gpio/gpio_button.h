/*
 * BeagleBone Black - GPIO Button Driver Header
 *
 * File:
 *     gpio_button.h
 */

#ifndef GPIO_BUTTON_H
#define GPIO_BUTTON_H


/* ------------------------------------------------------------------------- */
/* Driver Information                                                        */
/* ------------------------------------------------------------------------- */

#define GPIO_BUTTON_DRIVER_NAME       "bbb_gpio_button"

#define GPIO_BUTTON_CLASS_NAME        "bbb_gpio_button_class"

#define GPIO_BUTTON_DEVICE_NAME       "bbb_gpio_button"

#define GPIO_BUTTON_IRQ_NAME          "bbb_gpio_button_irq"


/* ------------------------------------------------------------------------- */
/* Device Tree GPIO Consumer                                                */
/* ------------------------------------------------------------------------- */

/*
 * Expected Device Tree property:
 *
 *     button-gpios = <...>;
 *
 * gpiod_get(NULL, "button", GPIOD_IN)
 * searches for the "button-gpios" property.
 */
#define GPIO_BUTTON_CONSUMER          "button"


/* ------------------------------------------------------------------------- */
/* Button Configuration                                                      */
/* ------------------------------------------------------------------------- */

#define GPIO_BUTTON_ACTIVE_LOW        1

#define GPIO_BUTTON_ACTIVE_HIGH       0


/* ------------------------------------------------------------------------- */
/* Driver Return Values                                                      */
/* ------------------------------------------------------------------------- */

#define GPIO_BUTTON_SUCCESS           0

#define GPIO_BUTTON_ERROR             -1


#endif /* GPIO_BUTTON_H */
