/*
 * BeagleBone Black - GPIO Interrupt Driver Header
 *
 * File:
 *     gpio_irq.h
 *
 * Purpose:
 *     Common definitions for the GPIO interrupt driver.
 */

#ifndef GPIO_IRQ_H
#define GPIO_IRQ_H

#include <linux/ioctl.h>


/* ------------------------------------------------------------------------- */
/* Driver Information                                                        */
/* ------------------------------------------------------------------------- */

#define GPIO_IRQ_DRIVER_NAME        "bbb_gpio_irq"

#define GPIO_IRQ_CLASS_NAME         "bbb_gpio_irq_class"

#define GPIO_IRQ_DEVICE_NAME        "bbb_gpio_irq"

#define GPIO_IRQ_NAME               "bbb_gpio_irq_handler"


/* ------------------------------------------------------------------------- */
/* Device Tree GPIO Consumer                                                */
/* ------------------------------------------------------------------------- */

/*
 * Expected Device Tree property:
 *
 *     irq-gpios = <...>;
 *
 * The following call:
 *
 *     gpiod_get(NULL, "irq", GPIOD_IN)
 *
 * looks for:
 *
 *     irq-gpios
 */
#define GPIO_IRQ_CONSUMER           "irq"


/* ------------------------------------------------------------------------- */
/* Interrupt Configuration                                                  */
/* ------------------------------------------------------------------------- */

/*
 * GPIO interrupt is configured for falling edge in gpio_irq.c.
 */
#define GPIO_IRQ_TRIGGER             "falling"


/* ------------------------------------------------------------------------- */
/* IOCTL Commands                                                            */
/* ------------------------------------------------------------------------- */

#define GPIO_IRQ_IOCTL_MAGIC        'G'


/*
 * Get current interrupt count.
 */
#define GPIO_IRQ_IOCTL_GET_COUNT    \
        _IOR(GPIO_IRQ_IOCTL_MAGIC, 0x01, int)


/*
 * Clear interrupt count.
 */
#define GPIO_IRQ_IOCTL_CLEAR_COUNT  \
        _IO(GPIO_IRQ_IOCTL_MAGIC, 0x02)


/* ------------------------------------------------------------------------- */
/* Driver Return Values                                                      */
/* ------------------------------------------------------------------------- */

#define GPIO_IRQ_SUCCESS            0

#define GPIO_IRQ_ERROR              -1


#endif /* GPIO_IRQ_H */
