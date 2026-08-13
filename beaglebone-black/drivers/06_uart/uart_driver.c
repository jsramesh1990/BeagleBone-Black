/*
 * BeagleBone Black - UART Driver
 *
 * File:
 *     uart_driver.c
 *
 * Purpose:
 *     Example UART platform driver demonstrating:
 *       - Platform driver probe/remove
 *       - Device Tree matching
 *       - UART register access
 *       - Character device interface
 *       - Basic TX/RX
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/ioport.h>

#include "uart_driver.h"


/* ------------------------------------------------------------------------- */
/* Driver Data                                                               */
/* ------------------------------------------------------------------------- */

static void __iomem *uart_base;

static resource_size_t uart_size;

static dev_t uart_dev;

static struct cdev uart_cdev;

static struct class *uart_class;

static struct device *uart_device;

static DEFINE_MUTEX(uart_mutex);


/* ------------------------------------------------------------------------- */
/* UART Register Access                                                      */
/* ------------------------------------------------------------------------- */

static inline u32 uart_read_reg(u32 offset)
{
    return readl(uart_base + offset);
}


static inline void uart_write_reg(u32 offset,
                                  u32 value)
{
    writel(value,
           uart_base + offset);
}


/* ------------------------------------------------------------------------- */
/* UART TX                                                                   */
/* ------------------------------------------------------------------------- */

static int uart_send_char(u8 data)
{
    unsigned int timeout = UART_TIMEOUT;

    /*
     * Wait until TX FIFO/register is ready.
     */
    while (!(uart_read_reg(UART_LSR_REG) &
             UART_LSR_THRE)) {

        if (--timeout == 0) {
            return -ETIMEDOUT;
        }

        cpu_relax();
    }

    /*
     * Write character to transmit register.
     */
    uart_write_reg(UART_THR_REG,
                   data);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* UART RX                                                                   */
/* ------------------------------------------------------------------------- */

static int uart_receive_char(u8 *data)
{
    unsigned int timeout = UART_TIMEOUT;

    /*
     * Wait for received data.
     */
    while (!(uart_read_reg(UART_LSR_REG) &
             UART_LSR_DATA_READY)) {

        if (--timeout == 0) {
            return -ETIMEDOUT;
        }

        cpu_relax();
    }

    *data =
        uart_read_reg(UART_RBR_REG) & 0xFF;

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Open                                                                      */
/* ------------------------------------------------------------------------- */

static int uart_open(struct inode *inode,
                     struct file *file)
{
    pr_info("%s: device opened\n",
            UART_DRIVER_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Write                                                                     */
/* ------------------------------------------------------------------------- */

static ssize_t uart_write(struct file *file,
                          const char __user *buffer,
                          size_t count,
                          loff_t *offset)
{
    u8 *data;

    size_t i;

    int ret;

    if (count == 0) {
        return 0;
    }

    data = kmalloc(count,
                   GFP_KERNEL);

    if (!data) {
        return -ENOMEM;
    }

    if (copy_from_user(data,
                       buffer,
                       count)) {

        kfree(data);

        return -EFAULT;
    }

    mutex_lock(&uart_mutex);

    for (i = 0; i < count; i++) {

        ret = uart_send_char(data[i]);

        if (ret) {

            mutex_unlock(&uart_mutex);

            kfree(data);

            return ret;
        }
    }

    mutex_unlock(&uart_mutex);

    kfree(data);

    return count;
}


/* ------------------------------------------------------------------------- */
/* Read                                                                      */
/* ------------------------------------------------------------------------- */

static ssize_t uart_read(struct file *file,
                         char __user *buffer,
                         size_t count,
                         loff_t *offset)
{
    u8 data;

    size_t i;

    int ret;

    if (count == 0) {
        return 0;
    }

    mutex_lock(&uart_mutex);

    for (i = 0; i < count; i++) {

        ret = uart_receive_char(&data);

        if (ret) {

            mutex_unlock(&uart_mutex);

            return (i > 0) ? i : ret;
        }

        if (copy_to_user(buffer + i,
                         &data,
                         1)) {

            mutex_unlock(&uart_mutex);

            return -EFAULT;
        }
    }

    mutex_unlock(&uart_mutex);

    return count;
}


/* ------------------------------------------------------------------------- */
/* Release                                                                   */
/* ------------------------------------------------------------------------- */

static int uart_release(struct inode *inode,
                        struct file *file)
{
    pr_info("%s: device closed\n",
            UART_DRIVER_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* File Operations                                                           */
/* ------------------------------------------------------------------------- */

static const struct file_operations uart_fops = {
    .owner   = THIS_MODULE,
    .open    = uart_open,
    .read    = uart_read,
    .write   = uart_write,
    .release = uart_release,
};


/* ------------------------------------------------------------------------- */
/* Hardware Initialization                                                   */
/* ------------------------------------------------------------------------- */

static int uart_hw_init(void)
{
    u32 value;

    /*
     * Example UART initialization.
     *
     * Actual register programming depends on
     * the UART IP block and SoC.
     */

    value = uart_read_reg(UART_LCR_REG);

    value |= UART_LCR_8N1;

    uart_write_reg(UART_LCR_REG,
                   value);

    pr_info("%s: UART hardware initialized\n",
            UART_DRIVER_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Probe                                                                     */
/* ------------------------------------------------------------------------- */

static int uart_probe(struct platform_device *pdev)
{
    struct resource *res;

    int ret;

    pr_info("%s: probing\n",
            UART_DRIVER_NAME);

    /*
     * Get UART memory resource from Device Tree.
     */
    res = platform_get_resource(pdev,
                                IORESOURCE_MEM,
                                0);

    if (!res) {

        dev_err(&pdev->dev,
                "UART memory resource not found\n");

        return -ENODEV;
    }

    uart_size =
        resource_size(res);

    /*
     * Map UART registers.
     */
    uart_base =
        devm_ioremap_resource(&pdev->dev,
                              res);

    if (IS_ERR(uart_base)) {

        ret = PTR_ERR(uart_base);

        dev_err(&pdev->dev,
                "Failed to map UART registers: %d\n",
                ret);

        return ret;
    }

    /*
     * Initialize UART hardware.
     */
    ret = uart_hw_init();

    if (ret) {
        return ret;
    }

    /*
     * Allocate character device number.
     */
    ret = alloc_chrdev_region(&uart_dev,
                              0,
                              1,
                              UART_DRIVER_NAME);

    if (ret < 0) {
        return ret;
    }

    /*
     * Initialize cdev.
     */
    cdev_init(&uart_cdev,
              &uart_fops);

    uart_cdev.owner = THIS_MODULE;

    ret = cdev_add(&uart_cdev,
                   uart_dev,
                   1);

    if (ret) {

        unregister_chrdev_region(uart_dev,
                                 1);

        return ret;
    }

    /*
     * Create device class.
     */
    uart_class =
        class_create(UART_CLASS_NAME);

    if (IS_ERR(uart_class)) {

        ret = PTR_ERR(uart_class);

        cdev_del(&uart_cdev);

        unregister_chrdev_region(uart_dev,
                                 1);

        return ret;
    }

    /*
     * Create /dev/bbb_uart.
     */
    uart_device =
        device_create(uart_class,
                      NULL,
                      uart_dev,
                      NULL,
                      UART_DEVICE_NAME);

    if (IS_ERR(uart_device)) {

        ret = PTR_ERR(uart_device);

        class_destroy(uart_class);

        cdev_del(&uart_cdev);

        unregister_chrdev_region(uart_dev,
                                 1);

        return ret;
    }

    pr_info("%s: UART mapped at %p\n",
            UART_DRIVER_NAME,
            uart_base);

    pr_info("%s: created /dev/%s\n",
            UART_DRIVER_NAME,
            UART_DEVICE_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Remove                                                                    */
/* ------------------------------------------------------------------------- */

static void uart_remove(struct platform_device *pdev)
{
    device_destroy(uart_class,
                   uart_dev);

    class_destroy(uart_class);

    cdev_del(&uart_cdev);

    unregister_chrdev_region(uart_dev,
                             1);

    uart_base = NULL;

    pr_info("%s: removed\n",
            UART_DRIVER_NAME);
}


/* ------------------------------------------------------------------------- */
/* Device Tree Match                                                         */
/* ------------------------------------------------------------------------- */

static const struct of_device_id uart_of_match[] = {
    {
        .compatible = "bbb,uart-test",
    },
    { }
};

MODULE_DEVICE_TABLE(of,
                    uart_of_match);


/* ------------------------------------------------------------------------- */
/* Platform Driver                                                           */
/* ------------------------------------------------------------------------- */

static struct platform_driver uart_platform_driver = {
    .probe  = uart_probe,
    .remove = uart_remove,

    .driver = {
        .name = UART_DRIVER_NAME,
        .of_match_table = uart_of_match,
    },
};


/* ------------------------------------------------------------------------- */
/* Module                                                                    */
/* ------------------------------------------------------------------------- */

module_platform_driver(uart_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Embedded Software Engineer");
MODULE_DESCRIPTION("BeagleBone Black UART Driver");
MODULE_VERSION("1.0");
