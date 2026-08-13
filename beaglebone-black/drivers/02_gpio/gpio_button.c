/*
 * BeagleBone Black - GPIO Button Driver
 *
 * File:
 *     gpio_button.c
 *
 * Purpose:
 *     GPIO button driver using the Linux GPIO descriptor API.
 *
 * The driver exposes:
 *     /dev/bbb_gpio_button
 *
 * Button press can be detected from user space using read()/poll().
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/uaccess.h>

#include "gpio_button.h"


static dev_t button_dev;
static struct cdev button_cdev;
static struct class *button_class;
static struct device *button_device;

static struct gpio_desc *button_gpio;
static int button_irq;

static wait_queue_head_t button_wait_queue;

static atomic_t button_event = ATOMIC_INIT(0);

static DEFINE_SPINLOCK(button_lock);


/* ------------------------------------------------------------------------- */
/* Interrupt Handler                                                         */
/* ------------------------------------------------------------------------- */

static irqreturn_t gpio_button_irq_handler(int irq, void *dev_id)
{
    unsigned long flags;

    spin_lock_irqsave(&button_lock, flags);

    atomic_set(&button_event, 1);

    spin_unlock_irqrestore(&button_lock, flags);

    wake_up_interruptible(&button_wait_queue);

    pr_info("%s: GPIO button interrupt received\n",
            GPIO_BUTTON_DRIVER_NAME);

    return IRQ_HANDLED;
}


/* ------------------------------------------------------------------------- */
/* Open                                                                      */
/* ------------------------------------------------------------------------- */

static int gpio_button_open(struct inode *inode,
                            struct file *file)
{
    pr_info("%s: device opened\n",
            GPIO_BUTTON_DRIVER_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Read                                                                      */
/* ------------------------------------------------------------------------- */

static ssize_t gpio_button_read(struct file *file,
                                char __user *buffer,
                                size_t count,
                                loff_t *offset)
{
    int value;

    if (count < sizeof(value)) {
        return -EINVAL;
    }

    /*
     * Wait until the interrupt handler reports an event.
     */
    if (wait_event_interruptible(button_wait_queue,
                                 atomic_read(&button_event))) {
        return -ERESTARTSYS;
    }

    value = gpiod_get_value(button_gpio);

    atomic_set(&button_event, 0);

    if (copy_to_user(buffer,
                     &value,
                     sizeof(value)) != 0) {
        return -EFAULT;
    }

    return sizeof(value);
}


/* ------------------------------------------------------------------------- */
/* Poll                                                                      */
/* ------------------------------------------------------------------------- */

static __poll_t gpio_button_poll(struct file *file,
                                 poll_table *wait)
{
    poll_wait(file,
              &button_wait_queue,
              wait);

    if (atomic_read(&button_event)) {
        return POLLIN | POLLRDNORM;
    }

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Release                                                                   */
/* ------------------------------------------------------------------------- */

static int gpio_button_release(struct inode *inode,
                               struct file *file)
{
    pr_info("%s: device closed\n",
            GPIO_BUTTON_DRIVER_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* File Operations                                                           */
/* ------------------------------------------------------------------------- */

static const struct file_operations gpio_button_fops = {
    .owner   = THIS_MODULE,
    .open    = gpio_button_open,
    .read    = gpio_button_read,
    .poll    = gpio_button_poll,
    .release = gpio_button_release,
};


/* ------------------------------------------------------------------------- */
/* Driver Initialization                                                     */
/* ------------------------------------------------------------------------- */

static int __init gpio_button_init(void)
{
    int ret;

    pr_info("%s: initializing\n",
            GPIO_BUTTON_DRIVER_NAME);

    init_waitqueue_head(&button_wait_queue);

    /*
     * Get GPIO from Device Tree.
     *
     * Property expected:
     *
     *     button-gpios = <...>;
     */
    button_gpio =
        gpiod_get(NULL,
                  GPIO_BUTTON_CONSUMER,
                  GPIOD_IN);

    if (IS_ERR(button_gpio)) {

        ret = PTR_ERR(button_gpio);

        pr_err("%s: failed to get GPIO: %d\n",
               GPIO_BUTTON_DRIVER_NAME,
               ret);

        return ret;
    }

    /*
     * Convert GPIO descriptor to IRQ number.
     */
    button_irq =
        gpiod_to_irq(button_gpio);

    if (button_irq < 0) {

        pr_err("%s: gpiod_to_irq failed\n",
               GPIO_BUTTON_DRIVER_NAME);

        gpiod_put(button_gpio);

        return button_irq;
    }

    /*
     * Request falling-edge interrupt.
     */
    ret = request_irq(button_irq,
                      gpio_button_irq_handler,
                      IRQF_TRIGGER_FALLING,
                      GPIO_BUTTON_IRQ_NAME,
                      NULL);

    if (ret) {

        pr_err("%s: request_irq failed: %d\n",
               GPIO_BUTTON_DRIVER_NAME,
               ret);

        gpiod_put(button_gpio);

        return ret;
    }

    /*
     * Allocate character device.
     */
    ret = alloc_chrdev_region(&button_dev,
                              0,
                              1,
                              GPIO_BUTTON_DRIVER_NAME);

    if (ret < 0) {

        free_irq(button_irq, NULL);
        gpiod_put(button_gpio);

        return ret;
    }

    cdev_init(&button_cdev,
              &gpio_button_fops);

    button_cdev.owner = THIS_MODULE;

    ret = cdev_add(&button_cdev,
                   button_dev,
                   1);

    if (ret) {

        unregister_chrdev_region(button_dev, 1);
        free_irq(button_irq, NULL);
        gpiod_put(button_gpio);

        return ret;
    }

    button_class =
        class_create(GPIO_BUTTON_CLASS_NAME);

    if (IS_ERR(button_class)) {

        ret = PTR_ERR(button_class);

        cdev_del(&button_cdev);
        unregister_chrdev_region(button_dev, 1);
        free_irq(button_irq, NULL);
        gpiod_put(button_gpio);

        return ret;
    }

    button_device =
        device_create(button_class,
                      NULL,
                      button_dev,
                      NULL,
                      GPIO_BUTTON_DEVICE_NAME);

    if (IS_ERR(button_device)) {

        ret = PTR_ERR(button_device);

        class_destroy(button_class);
        cdev_del(&button_cdev);
        unregister_chrdev_region(button_dev, 1);
        free_irq(button_irq, NULL);
        gpiod_put(button_gpio);

        return ret;
    }

    pr_info("%s: loaded successfully\n",
            GPIO_BUTTON_DRIVER_NAME);

    pr_info("%s: device /dev/%s\n",
            GPIO_BUTTON_DRIVER_NAME,
            GPIO_BUTTON_DEVICE_NAME);

    pr_info("%s: IRQ %d\n",
            GPIO_BUTTON_DRIVER_NAME,
            button_irq);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Driver Exit                                                               */
/* ------------------------------------------------------------------------- */

static void __exit gpio_button_exit(void)
{
    device_destroy(button_class,
                   button_dev);

    class_destroy(button_class);

    cdev_del(&button_cdev);

    unregister_chrdev_region(button_dev, 1);

    free_irq(button_irq, NULL);

    gpiod_put(button_gpio);

    pr_info("%s: unloaded\n",
            GPIO_BUTTON_DRIVER_NAME);
}


module_init(gpio_button_init);
module_exit(gpio_button_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Embedded Software Engineer");
MODULE_DESCRIPTION("BeagleBone Black GPIO Button Driver");
MODULE_VERSION("1.0");
