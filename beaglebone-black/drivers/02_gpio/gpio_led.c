/*
 * BeagleBone Black - GPIO LED Driver
 *
 * File:
 *     gpio_led.c
 *
 * Purpose:
 *     GPIO LED driver using the Linux GPIO descriptor API.
 *
 * The driver exposes:
 *
 *     /dev/bbb_gpio_led
 *
 * User space writes:
 *
 *     1 -> LED ON
 *     0 -> LED OFF
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/gpio/consumer.h>
#include <linux/uaccess.h>

#include "gpio_led.h"


static dev_t led_dev;
static struct cdev led_cdev;
static struct class *led_class;
static struct device *led_device;

static struct gpio_desc *led_gpio;


/* ------------------------------------------------------------------------- */
/* Open                                                                      */
/* ------------------------------------------------------------------------- */

static int gpio_led_open(struct inode *inode,
                         struct file *file)
{
    pr_info("%s: device opened\n",
            GPIO_LED_DRIVER_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Write                                                                     */
/* ------------------------------------------------------------------------- */

static ssize_t gpio_led_write(struct file *file,
                              const char __user *buffer,
                              size_t count,
                              loff_t *offset)
{
    char command;

    if (count < 1) {
        return -EINVAL;
    }

    if (copy_from_user(&command,
                       buffer,
                       sizeof(command)) != 0) {
        return -EFAULT;
    }

    switch (command) {

    case '1':
        gpiod_set_value(led_gpio, 1);

        pr_info("%s: LED ON\n",
                GPIO_LED_DRIVER_NAME);

        break;

    case '0':
        gpiod_set_value(led_gpio, 0);

        pr_info("%s: LED OFF\n",
                GPIO_LED_DRIVER_NAME);

        break;

    default:
        pr_warn("%s: invalid command '%c'\n",
                GPIO_LED_DRIVER_NAME,
                command);

        return -EINVAL;
    }

    return count;
}


/* ------------------------------------------------------------------------- */
/* Read                                                                      */
/* ------------------------------------------------------------------------- */

static ssize_t gpio_led_read(struct file *file,
                             char __user *buffer,
                             size_t count,
                             loff_t *offset)
{
    char value;

    if (*offset != 0) {
        return 0;
    }

    if (count < 2) {
        return -EINVAL;
    }

    if (gpiod_get_value(led_gpio)) {
        value = '1';
    } else {
        value = '0';
    }

    if (copy_to_user(buffer,
                     &value,
                     sizeof(value)) != 0) {
        return -EFAULT;
    }

    *offset = 1;

    return 1;
}


/* ------------------------------------------------------------------------- */
/* Release                                                                   */
/* ------------------------------------------------------------------------- */

static int gpio_led_release(struct inode *inode,
                            struct file *file)
{
    pr_info("%s: device closed\n",
            GPIO_LED_DRIVER_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* File Operations                                                           */
/* ------------------------------------------------------------------------- */

static const struct file_operations gpio_led_fops = {
    .owner   = THIS_MODULE,
    .open    = gpio_led_open,
    .read    = gpio_led_read,
    .write   = gpio_led_write,
    .release = gpio_led_release,
};


/* ------------------------------------------------------------------------- */
/* Initialization                                                            */
/* ------------------------------------------------------------------------- */

static int __init gpio_led_init(void)
{
    int ret;

    pr_info("%s: initializing\n",
            GPIO_LED_DRIVER_NAME);

    /*
     * Obtain LED GPIO from Device Tree.
     */
    led_gpio =
        gpiod_get(NULL,
                  GPIO_LED_CONSUMER,
                  GPIOD_OUT_LOW);

    if (IS_ERR(led_gpio)) {

        ret = PTR_ERR(led_gpio);

        pr_err("%s: failed to get GPIO: %d\n",
               GPIO_LED_DRIVER_NAME,
               ret);

        return ret;
    }

    /*
     * Allocate character-device number.
     */
    ret = alloc_chrdev_region(&led_dev,
                              0,
                              1,
                              GPIO_LED_DRIVER_NAME);

    if (ret < 0) {

        gpiod_put(led_gpio);

        return ret;
    }

    /*
     * Initialize cdev.
     */
    cdev_init(&led_cdev,
              &gpio_led_fops);

    led_cdev.owner = THIS_MODULE;

    ret = cdev_add(&led_cdev,
                   led_dev,
                   1);

    if (ret) {

        unregister_chrdev_region(led_dev, 1);
        gpiod_put(led_gpio);

        return ret;
    }

    /*
     * Create class.
     */
    led_class =
        class_create(GPIO_LED_CLASS_NAME);

    if (IS_ERR(led_class)) {

        ret = PTR_ERR(led_class);

        cdev_del(&led_cdev);
        unregister_chrdev_region(led_dev, 1);
        gpiod_put(led_gpio);

        return ret;
    }

    /*
     * Create /dev/bbb_gpio_led.
     */
    led_device =
        device_create(led_class,
                      NULL,
                      led_dev,
                      NULL,
                      GPIO_LED_DEVICE_NAME);

    if (IS_ERR(led_device)) {

        ret = PTR_ERR(led_device);

        class_destroy(led_class);
        cdev_del(&led_cdev);
        unregister_chrdev_region(led_dev, 1);
        gpiod_put(led_gpio);

        return ret;
    }

    pr_info("%s: loaded successfully\n",
            GPIO_LED_DRIVER_NAME);

    pr_info("%s: device /dev/%s\n",
            GPIO_LED_DRIVER_NAME,
            GPIO_LED_DEVICE_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Exit                                                                      */
/* ------------------------------------------------------------------------- */

static void __exit gpio_led_exit(void)
{
    /*
     * Turn LED OFF before removing driver.
     */
    gpiod_set_value(led_gpio, 0);

    device_destroy(led_class,
                   led_dev);

    class_destroy(led_class);

    cdev_del(&led_cdev);

    unregister_chrdev_region(led_dev, 1);

    gpiod_put(led_gpio);

    pr_info("%s: unloaded\n",
            GPIO_LED_DRIVER_NAME);
}


module_init(gpio_led_init);
module_exit(gpio_led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Embedded Software Engineer");
MODULE_DESCRIPTION("BeagleBone Black GPIO LED Driver");
MODULE_VERSION("1.0");
