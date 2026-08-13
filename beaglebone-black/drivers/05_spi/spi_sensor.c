/*
 * BeagleBone Black - SPI Sensor Driver
 *
 * File:
 *     spi_sensor.c
 *
 * Purpose:
 *     Example SPI sensor client driver using the Linux SPI framework.
 *
 * Device:
 *     /dev/bbb_spi_sensor
 *
 * Demonstrates:
 *     - SPI probe/remove
 *     - SPI register read
 *     - SPI register write
 *     - SPI message transfer
 *     - User-space read/write
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

#include "spi_sensor.h"


/* ------------------------------------------------------------------------- */
/* Driver Data                                                               */
/* ------------------------------------------------------------------------- */

static struct spi_device *sensor_spi;

static dev_t sensor_dev;

static struct cdev sensor_cdev;

static struct class *sensor_class;

static struct device *sensor_device;

static DEFINE_MUTEX(sensor_mutex);


/* ------------------------------------------------------------------------- */
/* Register Read                                                             */
/* ------------------------------------------------------------------------- */

static int sensor_read_register(u8 reg,
                                u8 *value)
{
    u8 tx_buffer[2];

    u8 rx_buffer[2];

    struct spi_transfer transfer = {
        .tx_buf = tx_buffer,
        .rx_buf = rx_buffer,
        .len = 2,
    };

    struct spi_message message;

    int ret;

    /*
     * Set read bit according to the example sensor protocol.
     */
    tx_buffer[0] =
        reg | SPI_SENSOR_READ_BIT;

    tx_buffer[1] = 0x00;

    rx_buffer[0] = 0x00;
    rx_buffer[1] = 0x00;

    spi_message_init(&message);

    spi_message_add_tail(&transfer,
                         &message);

    ret = spi_sync(sensor_spi,
                   &message);

    if (ret < 0) {
        return ret;
    }

    *value = rx_buffer[1];

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Register Write                                                            */
/* ------------------------------------------------------------------------- */

static int sensor_write_register(u8 reg,
                                 u8 value)
{
    u8 tx_buffer[2];

    struct spi_transfer transfer = {
        .tx_buf = tx_buffer,
        .len = 2,
    };

    struct spi_message message;

    int ret;

    tx_buffer[0] =
        reg & ~SPI_SENSOR_READ_BIT;

    tx_buffer[1] =
        value;

    spi_message_init(&message);

    spi_message_add_tail(&transfer,
                         &message);

    ret = spi_sync(sensor_spi,
                   &message);

    return ret;
}


/* ------------------------------------------------------------------------- */
/* Sensor Value Read                                                         */
/* ------------------------------------------------------------------------- */

static int sensor_read_value(u16 *value)
{
    u8 high;
    u8 low;

    int ret;

    ret = sensor_read_register(
        SPI_SENSOR_DATA_HIGH,
        &high);

    if (ret) {
        return ret;
    }

    ret = sensor_read_register(
        SPI_SENSOR_DATA_LOW,
        &low);

    if (ret) {
        return ret;
    }

    *value =
        ((u16)high << 8) |
        low;

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Open                                                                      */
/* ------------------------------------------------------------------------- */

static int sensor_open(struct inode *inode,
                       struct file *file)
{
    pr_info("%s: opened\n",
            SPI_SENSOR_DRIVER_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Read                                                                      */
/* ------------------------------------------------------------------------- */

static ssize_t sensor_read(struct file *file,
                           char __user *buffer,
                           size_t count,
                           loff_t *offset)
{
    u16 value;

    int ret;

    if (count < sizeof(value)) {
        return -EINVAL;
    }

    mutex_lock(&sensor_mutex);

    ret = sensor_read_value(&value);

    mutex_unlock(&sensor_mutex);

    if (ret) {
        return ret;
    }

    if (copy_to_user(buffer,
                     &value,
                     sizeof(value))) {

        return -EFAULT;
    }

    return sizeof(value);
}


/* ------------------------------------------------------------------------- */
/* Write                                                                     */
/* ------------------------------------------------------------------------- */

static ssize_t sensor_write(struct file *file,
                            const char __user *buffer,
                            size_t count,
                            loff_t *offset)
{
    struct spi_sensor_write_data data;

    int ret;

    if (count != sizeof(data)) {
        return -EINVAL;
    }

    if (copy_from_user(&data,
                       buffer,
                       sizeof(data))) {

        return -EFAULT;
    }

    mutex_lock(&sensor_mutex);

    ret = sensor_write_register(data.reg,
                                data.value);

    mutex_unlock(&sensor_mutex);

    if (ret) {
        return ret;
    }

    return sizeof(data);
}


/* ------------------------------------------------------------------------- */
/* Release                                                                   */
/* ------------------------------------------------------------------------- */

static int sensor_release(struct inode *inode,
                          struct file *file)
{
    pr_info("%s: closed\n",
            SPI_SENSOR_DRIVER_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* File Operations                                                           */
/* ------------------------------------------------------------------------- */

static const struct file_operations sensor_fops = {
    .owner   = THIS_MODULE,
    .open    = sensor_open,
    .read    = sensor_read,
    .write   = sensor_write,
    .release = sensor_release,
};


/* ------------------------------------------------------------------------- */
/* Probe                                                                     */
/* ------------------------------------------------------------------------- */

static int spi_sensor_probe(struct spi_device *spi)
{
    int ret;

    u8 device_id;

    sensor_spi = spi;

    /*
     * Configure SPI.
     */
    spi->mode =
        SPI_SENSOR_MODE;

    spi->bits_per_word =
        SPI_SENSOR_BITS_PER_WORD;

    ret = spi_setup(spi);

    if (ret) {

        pr_err("%s: spi_setup failed: %d\n",
               SPI_SENSOR_DRIVER_NAME,
               ret);

        return ret;
    }

    pr_info("%s: SPI sensor detected\n",
            SPI_SENSOR_DRIVER_NAME);

    pr_info("%s: bus=%d cs=%d speed=%u Hz\n",
            SPI_SENSOR_DRIVER_NAME,
            spi->controller->bus_num,
            spi->chip_select,
            spi->max_speed_hz);

    /*
     * Read sensor ID.
     */
    ret = sensor_read_register(
        SPI_SENSOR_WHO_AM_I,
        &device_id);

    if (ret) {

        pr_err("%s: sensor ID read failed: %d\n",
               SPI_SENSOR_DRIVER_NAME,
               ret);

        return ret;
    }

    pr_info("%s: WHO_AM_I = 0x%02X\n",
            SPI_SENSOR_DRIVER_NAME,
            device_id);

    /*
     * Create character device.
     */
    ret = alloc_chrdev_region(&sensor_dev,
                              0,
                              1,
                              SPI_SENSOR_DRIVER_NAME);

    if (ret < 0) {
        return ret;
    }

    cdev_init(&sensor_cdev,
              &sensor_fops);

    sensor_cdev.owner = THIS_MODULE;

    ret = cdev_add(&sensor_cdev,
                   sensor_dev,
                   1);

    if (ret) {

        unregister_chrdev_region(sensor_dev,
                                 1);

        return ret;
    }

    sensor_class =
        class_create(SPI_SENSOR_CLASS_NAME);

    if (IS_ERR(sensor_class)) {

        ret = PTR_ERR(sensor_class);

        cdev_del(&sensor_cdev);

        unregister_chrdev_region(sensor_dev,
                                 1);

        return ret;
    }

    sensor_device =
        device_create(sensor_class,
                      NULL,
                      sensor_dev,
                      NULL,
                      SPI_SENSOR_DEVICE_NAME);

    if (IS_ERR(sensor_device)) {

        ret = PTR_ERR(sensor_device);

        class_destroy(sensor_class);

        cdev_del(&sensor_cdev);

        unregister_chrdev_region(sensor_dev,
                                 1);

        return ret;
    }

    pr_info("%s: created /dev/%s\n",
            SPI_SENSOR_DRIVER_NAME,
            SPI_SENSOR_DEVICE_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Remove                                                                    */
/* ------------------------------------------------------------------------- */

static void spi_sensor_remove(struct spi_device *spi)
{
    device_destroy(sensor_class,
                   sensor_dev);

    class_destroy(sensor_class);

    cdev_del(&sensor_cdev);

    unregister_chrdev_region(sensor_dev,
                             1);

    sensor_spi = NULL;

    pr_info("%s: removed\n",
            SPI_SENSOR_DRIVER_NAME);
}


/* ------------------------------------------------------------------------- */
/* SPI Device ID                                                             */
/* ------------------------------------------------------------------------- */

static const struct spi_device_id spi_sensor_ids[] = {
    {
        SPI_SENSOR_DEVICE_NAME,
        0
    },
    { }
};

MODULE_DEVICE_TABLE(spi,
                    spi_sensor_ids);


/* ------------------------------------------------------------------------- */
/* SPI Driver                                                                */
/* ------------------------------------------------------------------------- */

static struct spi_driver spi_sensor_driver = {
    .driver = {
        .name = SPI_SENSOR_DRIVER_NAME,
    },

    .probe = spi_sensor_probe,

    .remove = spi_sensor_remove,

    .id_table = spi_sensor_ids,
};


/* ------------------------------------------------------------------------- */
/* Module                                                                    */
/* ------------------------------------------------------------------------- */

module_spi_driver(spi_sensor_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Embedded Software Engineer");
MODULE_DESCRIPTION("BeagleBone Black SPI Sensor Driver");
MODULE_VERSION("1.0");
