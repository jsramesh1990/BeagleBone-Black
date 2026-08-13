/*
 * BeagleBone Black - SPI Display Driver
 *
 * File:
 *     spi_display.c
 *
 * Purpose:
 *     Example SPI display client driver using the Linux SPI framework.
 *
 * Device:
 *     /dev/bbb_spi_display
 *
 * This example demonstrates:
 *     - SPI probe/remove
 *     - SPI transfer
 *     - Display command/data handling
 *     - Character device interface
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spi/spi.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

#include "spi_display.h"


/* ------------------------------------------------------------------------- */
/* Driver Data                                                               */
/* ------------------------------------------------------------------------- */

static struct spi_device *display_spi;

static dev_t display_dev;

static struct cdev display_cdev;

static struct class *display_class;

static struct device *display_device;

static DEFINE_MUTEX(display_mutex);


/* ------------------------------------------------------------------------- */
/* SPI Transfer                                                              */
/* ------------------------------------------------------------------------- */

static int display_spi_write(const u8 *data,
                             size_t length)
{
    struct spi_transfer transfer = {
        .tx_buf = data,
        .len = length,
    };

    struct spi_message message;

    int ret;

    spi_message_init(&message);

    spi_message_add_tail(&transfer,
                         &message);

    ret = spi_sync(display_spi,
                   &message);

    if (ret < 0) {

        pr_err("%s: SPI transfer failed: %d\n",
               SPI_DISPLAY_DRIVER_NAME,
               ret);

        return ret;
    }

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Display Command                                                           */
/* ------------------------------------------------------------------------- */

static int display_send_command(u8 command)
{
    u8 packet[2];

    /*
     * First byte:
     *     0 = command
     *
     * Second byte:
     *     command value
     */
    packet[0] = SPI_DISPLAY_COMMAND;
    packet[1] = command;

    return display_spi_write(packet,
                             sizeof(packet));
}


/* ------------------------------------------------------------------------- */
/* Display Data                                                              */
/* ------------------------------------------------------------------------- */

static int display_send_data(const u8 *data,
                             size_t length)
{
    u8 *packet;

    int ret;

    packet = kmalloc(length + 1,
                     GFP_KERNEL);

    if (!packet) {
        return -ENOMEM;
    }

    /*
     * First byte:
     *     1 = display data
     */
    packet[0] = SPI_DISPLAY_DATA;

    memcpy(&packet[1],
           data,
           length);

    ret = display_spi_write(packet,
                            length + 1);

    kfree(packet);

    return ret;
}


/* ------------------------------------------------------------------------- */
/* Display Initialization                                                    */
/* ------------------------------------------------------------------------- */

static int display_initialize(void)
{
    int ret;

    /*
     * Example initialization sequence.
     *
     * Actual commands depend on the display controller.
     */

    ret = display_send_command(
        SPI_DISPLAY_CMD_RESET);

    if (ret) {
        return ret;
    }

    msleep(20);

    ret = display_send_command(
        SPI_DISPLAY_CMD_DISPLAY_ON);

    if (ret) {
        return ret;
    }

    pr_info("%s: display initialized\n",
            SPI_DISPLAY_DRIVER_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Open                                                                      */
/* ------------------------------------------------------------------------- */

static int display_open(struct inode *inode,
                        struct file *file)
{
    pr_info("%s: opened\n",
            SPI_DISPLAY_DRIVER_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Write                                                                     */
/* ------------------------------------------------------------------------- */

static ssize_t display_write(struct file *file,
                             const char __user *buffer,
                             size_t count,
                             loff_t *offset)
{
    u8 *data;

    int ret;

    if (count == 0) {
        return 0;
    }

    if (count > SPI_DISPLAY_MAX_TRANSFER) {
        return -EMSGSIZE;
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

    mutex_lock(&display_mutex);

    ret = display_send_data(data,
                            count);

    mutex_unlock(&display_mutex);

    kfree(data);

    if (ret) {
        return ret;
    }

    return count;
}


/* ------------------------------------------------------------------------- */
/* Release                                                                   */
/* ------------------------------------------------------------------------- */

static int display_release(struct inode *inode,
                           struct file *file)
{
    pr_info("%s: closed\n",
            SPI_DISPLAY_DRIVER_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* File Operations                                                           */
/* ------------------------------------------------------------------------- */

static const struct file_operations display_fops = {
    .owner   = THIS_MODULE,
    .open    = display_open,
    .write   = display_write,
    .release = display_release,
};


/* ------------------------------------------------------------------------- */
/* Probe                                                                     */
/* ------------------------------------------------------------------------- */

static int spi_display_probe(struct spi_device *spi)
{
    int ret;

    display_spi = spi;

    /*
     * SPI configuration.
     */
    spi->mode = SPI_DISPLAY_MODE;

    spi->bits_per_word =
        SPI_DISPLAY_BITS_PER_WORD;

    ret = spi_setup(spi);

    if (ret) {

        pr_err("%s: spi_setup failed: %d\n",
               SPI_DISPLAY_DRIVER_NAME,
               ret);

        return ret;
    }

    pr_info("%s: SPI display detected\n",
            SPI_DISPLAY_DRIVER_NAME);

    pr_info("%s: bus=%d cs=%d speed=%u Hz\n",
            SPI_DISPLAY_DRIVER_NAME,
            spi->controller->bus_num,
            spi->chip_select,
            spi->max_speed_hz);

    /*
     * Initialize display.
     */
    ret = display_initialize();

    if (ret) {
        return ret;
    }

    /*
     * Allocate character device.
     */
    ret = alloc_chrdev_region(&display_dev,
                              0,
                              1,
                              SPI_DISPLAY_DRIVER_NAME);

    if (ret < 0) {
        return ret;
    }

    cdev_init(&display_cdev,
              &display_fops);

    display_cdev.owner = THIS_MODULE;

    ret = cdev_add(&display_cdev,
                   display_dev,
                   1);

    if (ret) {

        unregister_chrdev_region(display_dev,
                                 1);

        return ret;
    }

    display_class =
        class_create(SPI_DISPLAY_CLASS_NAME);

    if (IS_ERR(display_class)) {

        ret = PTR_ERR(display_class);

        cdev_del(&display_cdev);

        unregister_chrdev_region(display_dev,
                                 1);

        return ret;
    }

    display_device =
        device_create(display_class,
                      NULL,
                      display_dev,
                      NULL,
                      SPI_DISPLAY_DEVICE_NAME);

    if (IS_ERR(display_device)) {

        ret = PTR_ERR(display_device);

        class_destroy(display_class);

        cdev_del(&display_cdev);

        unregister_chrdev_region(display_dev,
                                 1);

        return ret;
    }

    pr_info("%s: created /dev/%s\n",
            SPI_DISPLAY_DRIVER_NAME,
            SPI_DISPLAY_DEVICE_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Remove                                                                    */
/* ------------------------------------------------------------------------- */

static void spi_display_remove(struct spi_device *spi)
{
    device_destroy(display_class,
                   display_dev);

    class_destroy(display_class);

    cdev_del(&display_cdev);

    unregister_chrdev_region(display_dev,
                             1);

    display_spi = NULL;

    pr_info("%s: removed\n",
            SPI_DISPLAY_DRIVER_NAME);
}


/* ------------------------------------------------------------------------- */
/* SPI Device ID                                                             */
/* ------------------------------------------------------------------------- */

static const struct spi_device_id spi_display_ids[] = {
    {
        SPI_DISPLAY_DEVICE_NAME,
        0
    },
    { }
};

MODULE_DEVICE_TABLE(spi,
                    spi_display_ids);


/* ------------------------------------------------------------------------- */
/* SPI Driver                                                                */
/* ------------------------------------------------------------------------- */

static struct spi_driver spi_display_driver = {
    .driver = {
        .name = SPI_DISPLAY_DRIVER_NAME,
    },

    .probe = spi_display_probe,

    .remove = spi_display_remove,

    .id_table = spi_display_ids,
};


/* ------------------------------------------------------------------------- */
/* Module                                                                    */
/* ------------------------------------------------------------------------- */

module_spi_driver(spi_display_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Embedded Software Engineer");
MODULE_DESCRIPTION("BeagleBone Black SPI Display Driver");
MODULE_VERSION("1.0");
