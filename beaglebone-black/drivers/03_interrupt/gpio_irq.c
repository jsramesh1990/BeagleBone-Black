/*
 * BeagleBone Black - GPIO Interrupt Driver
 *
 * File:
 *     gpio_irq.c
 *
 * Purpose:
 *     Demonstrates GPIO interrupt handling in a Linux kernel driver.
 *
 * Device:
 *     /dev/bbb_gpio_irq
 *
 * Flow:
 *
 *     GPIO Hardware
 *          |
 *          v
 *     GPIO Controller
 *          |
 *          v
 *     Linux IRQ Subsystem
 *          |
 *          v
 *     gpio_irq_handler()
 *          |
 *          v
 *     wake_up_interruptible()
 *          |
 *          v
 *     User Space read()/poll()
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/uaccess.h>
#include <linux/atomic.h>

#include "gpio_irq.h"


/* ------------------------------------------------------------------------- */
/* Global Driver Data                                                        */
/* ------------------------------------------------------------------------- */

static dev_t gpio_irq_dev;

static struct cdev gpio_irq_cdev;

static struct class *gpio_irq_class;

static struct device *gpio_irq_device;

static struct gpio_desc *gpio_irq_desc;

static int gpio_irq_number;

static wait_queue_head_t gpio_irq_wait_queue;

static atomic_t irq_event = ATOMIC_INIT(0);

static atomic_t irq_count = ATOMIC_INIT(0);


/* ------------------------------------------------------------------------- */
/* GPIO Interrupt Handler                                                    */
/* ------------------------------------------------------------------------- */

static irqreturn_t gpio_irq_handler(int irq,
                                    void *dev_id)
{
    /*
     * Increment interrupt counter.
     */
    atomic_inc(&irq_count);

    /*
     * Notify user-space readers.
     */
    atomic_set(&irq_event, 1);

    wake_up_interruptible(&gpio_irq_wait_queue);

    pr_info("%s: GPIO interrupt received, count=%d\n",
            GPIO_IRQ_DRIVER_NAME,
            atomic_read(&irq_count));

    return IRQ_HANDLED;
}


/* ------------------------------------------------------------------------- */
/* Open                                                                      */
/* ------------------------------------------------------------------------- */

static int gpio_irq_open(struct inode *inode,
                         struct file *file)
{
    pr_info("%s: device opened\n",
            GPIO_IRQ_DRIVER_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Read                                                                      */
/* ------------------------------------------------------------------------- */

static ssize_t gpio_irq_read(struct file *file,
                             char __user *buffer,
                             size_t count,
                             loff_t *offset)
{
    int event_count;

    /*
     * User must provide enough space for an integer.
     */
    if (count < sizeof(event_count)) {
        return -EINVAL;
    }

    /*
     * Block until an interrupt occurs.
     */
    if (wait_event_interruptible(
            gpio_irq_wait_queue,
            atomic_read(&irq_event))) {

        return -ERESTARTSYS;
    }

    /*
     * Get current interrupt count.
     */
    event_count =
        atomic_read(&irq_count);

    /*
     * Clear event flag.
     */
    atomic_set(&irq_event, 0);

    /*
     * Copy interrupt count to user space.
     */
    if (copy_to_user(buffer,
                     &event_count,
                     sizeof(event_count)) != 0) {

        return -EFAULT;
    }

    return sizeof(event_count);
}


/* ------------------------------------------------------------------------- */
/* Poll                                                                      */
/* ------------------------------------------------------------------------- */

static __poll_t gpio_irq_poll(struct file *file,
                              poll_table *wait)
{
    /*
     * Add process to wait queue.
     */
    poll_wait(file,
              &gpio_irq_wait_queue,
              wait);

    /*
     * Report data available when interrupt occurs.
     */
    if (atomic_read(&irq_event)) {
        return POLLIN | POLLRDNORM;
    }

    return 0;
}


/* ------------------------------------------------------------------------- */
/* IOCTL                                                                     */
/* ------------------------------------------------------------------------- */

static long gpio_irq_ioctl(struct file *file,
                           unsigned int cmd,
                           unsigned long arg)
{
    int count;

    switch (cmd) {

    case GPIO_IRQ_IOCTL_GET_COUNT:

        count = atomic_read(&irq_count);

        if (copy_to_user((int __user *)arg,
                         &count,
                         sizeof(count)) != 0) {

            return -EFAULT;
        }

        break;


    case GPIO_IRQ_IOCTL_CLEAR_COUNT:

        atomic_set(&irq_count, 0);

        pr_info("%s: interrupt count cleared\n",
                GPIO_IRQ_DRIVER_NAME);

        break;


    default:

        pr_warn("%s: unsupported ioctl 0x%x\n",
                GPIO_IRQ_DRIVER_NAME,
                cmd);

        return -ENOTTY;
    }

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Release                                                                   */
/* ------------------------------------------------------------------------- */

static int gpio_irq_release(struct inode *inode,
                            struct file *file)
{
    pr_info("%s: device closed\n",
            GPIO_IRQ_DRIVER_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* File Operations                                                           */
/* ------------------------------------------------------------------------- */

static const struct file_operations gpio_irq_fops = {
    .owner          = THIS_MODULE,
    .open           = gpio_irq_open,
    .read           = gpio_irq_read,
    .poll           = gpio_irq_poll,
    .unlocked_ioctl = gpio_irq_ioctl,
    .release        = gpio_irq_release,
};


/* ------------------------------------------------------------------------- */
/* Driver Initialization                                                     */
/* ------------------------------------------------------------------------- */

static int __init gpio_irq_init(void)
{
    int ret;

    pr_info("%s: initializing\n",
            GPIO_IRQ_DRIVER_NAME);

    /*
     * Initialize wait queue.
     */
    init_waitqueue_head(&gpio_irq_wait_queue);

    /*
     * Obtain GPIO from Device Tree.
     *
     * Expected property:
     *
     *     irq-gpios = <...>;
     */
    gpio_irq_desc =
        gpiod_get(NULL,
                  GPIO_IRQ_CONSUMER,
                  GPIOD_IN);

    if (IS_ERR(gpio_irq_desc)) {

        ret = PTR_ERR(gpio_irq_desc);

        pr_err("%s: failed to get GPIO: %d\n",
               GPIO_IRQ_DRIVER_NAME,
               ret);

        return ret;
    }

    /*
     * Convert GPIO descriptor to Linux IRQ number.
     */
    gpio_irq_number =
        gpiod_to_irq(gpio_irq_desc);

    if (gpio_irq_number < 0) {

        pr_err("%s: gpiod_to_irq failed: %d\n",
               GPIO_IRQ_DRIVER_NAME,
               gpio_irq_number);

        gpiod_put(gpio_irq_desc);

        return gpio_irq_number;
    }

    pr_info("%s: GPIO IRQ number = %d\n",
            GPIO_IRQ_DRIVER_NAME,
            gpio_irq_number);

    /*
     * Request falling-edge GPIO interrupt.
     *
     * Change IRQF_TRIGGER_FALLING to:
     *
     *     IRQF_TRIGGER_RISING
     *
     * or:
     *
     *     IRQF_TRIGGER_RISING |
     *     IRQF_TRIGGER_FALLING
     *
     * depending on the hardware.
     */
    ret = request_irq(gpio_irq_number,
                      gpio_irq_handler,
                      IRQF_TRIGGER_FALLING,
                      GPIO_IRQ_NAME,
                      NULL);

    if (ret) {

        pr_err("%s: request_irq failed: %d\n",
               GPIO_IRQ_DRIVER_NAME,
               ret);

        gpiod_put(gpio_irq_desc);

        return ret;
    }

    /*
     * Allocate character-device number.
     */
    ret = alloc_chrdev_region(&gpio_irq_dev,
                              0,
                              1,
                              GPIO_IRQ_DRIVER_NAME);

    if (ret < 0) {

        free_irq(gpio_irq_number, NULL);
        gpiod_put(gpio_irq_desc);

        return ret;
    }

    /*
     * Initialize cdev.
     */
    cdev_init(&gpio_irq_cdev,
              &gpio_irq_fops);

    gpio_irq_cdev.owner = THIS_MODULE;

    /*
     * Add cdev.
     */
    ret = cdev_add(&gpio_irq_cdev,
                   gpio_irq_dev,
                   1);

    if (ret) {

        unregister_chrdev_region(gpio_irq_dev, 1);
        free_irq(gpio_irq_number, NULL);
        gpiod_put(gpio_irq_desc);

        return ret;
    }

    /*
     * Create device class.
     */
    gpio_irq_class =
        class_create(GPIO_IRQ_CLASS_NAME);

    if (IS_ERR(gpio_irq_class)) {

        ret = PTR_ERR(gpio_irq_class);

        cdev_del(&gpio_irq_cdev);
        unregister_chrdev_region(gpio_irq_dev, 1);
        free_irq(gpio_irq_number, NULL);
        gpiod_put(gpio_irq_desc);

        return ret;
    }

    /*
     * Create /dev/bbb_gpio_irq.
     */
    gpio_irq_device =
        device_create(gpio_irq_class,
                      NULL,
                      gpio_irq_dev,
                      NULL,
                      GPIO_IRQ_DEVICE_NAME);

    if (IS_ERR(gpio_irq_device)) {

        ret = PTR_ERR(gpio_irq_device);

        class_destroy(gpio_irq_class);
        cdev_del(&gpio_irq_cdev);
        unregister_chrdev_region(gpio_irq_dev, 1);
        free_irq(gpio_irq_number, NULL);
        gpiod_put(gpio_irq_desc);

        return ret;
    }

    pr_info("%s: driver loaded successfully\n",
            GPIO_IRQ_DRIVER_NAME);

    pr_info("%s: device /dev/%s\n",
            GPIO_IRQ_DRIVER_NAME,
            GPIO_IRQ_DEVICE_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Driver Exit                                                               */
/* ------------------------------------------------------------------------- */

static void __exit gpio_irq_exit(void)
{
    pr_info("%s: removing driver\n",
            GPIO_IRQ_DRIVER_NAME);

    /*
     * Remove device.
     */
    device_destroy(gpio_irq_class,
                   gpio_irq_dev);

    /*
     * Remove class.
     */
    class_destroy(gpio_irq_class);

    /*
     * Remove character device.
     */
    cdev_del(&gpio_irq_cdev);

    /*
     * Release device number.
     */
    unregister_chrdev_region(gpio_irq_dev,
                             1);

    /*
     * Free IRQ.
     */
    free_irq(gpio_irq_number,
             NULL);

    /*
     * Release GPIO descriptor.
     */
    gpiod_put(gpio_irq_desc);

    pr_info("%s: driver unloaded\n",
            GPIO_IRQ_DRIVER_NAME);
}


/* ------------------------------------------------------------------------- */
/* Module Registration                                                       */
/* ------------------------------------------------------------------------- */

module_init(gpio_irq_init);

module_exit(gpio_irq_exit);


/* ------------------------------------------------------------------------- */
/* Module Information                                                        */
/* ------------------------------------------------------------------------- */

MODULE_LICENSE("GPL");

MODULE_AUTHOR("Embedded Software Engineer");

MODULE_DESCRIPTION("BeagleBone Black GPIO Interrupt Driver");

MODULE_VERSION("1.0");
