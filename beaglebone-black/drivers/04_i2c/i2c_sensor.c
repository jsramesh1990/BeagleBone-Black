/*
 * BeagleBone Black - I2C Sensor Driver
 *
 * File:
 *     i2c_sensor.c
 *
 * Purpose:
 *     Example I2C sensor client driver demonstrating:
 *       - I2C probe/remove
 *       - Register read
 *       - Register write
 *       - User-space read interface
 *
 * Device:
 *     /dev/bbb_i2c_sensor
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

#include "i2c_sensor.h"


/* ------------------------------------------------------------------------- */
/* Driver Data                                                               */
/* ------------------------------------------------------------------------- */

static struct i2c_client *sensor_client;

static dev_t sensor_dev;

static struct cdev sensor_cdev;

static struct class *sensor_class;

static struct device *sensor_device;

static DEFINE_MUTEX(sensor_mutex);


/* ------------------------------------------------------------------------- */
/* Register Read                                                             */
/* ------------------------------------------------------------------------- */

static int sensor_read_reg(u8 reg,
                           u8 *value)
{
    int ret;

    ret = i2c_smbus_read_byte_data(sensor_client,
                                   reg);

    if (ret < 0) {
        return ret;
    }

    *value = ret & 0xFF;

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Register Write                                                            */
/* ------------------------------------------------------------------------- */

static int sensor_write_reg(u8 reg,
                            u8 value)
{
    return i2c_smbus_write_byte_data(sensor_client,
                                     reg,
                                     value);
}


/* ------------------------------------------------------------------------- */
/* Sensor Read                                                               */
/* ------------------------------------------------------------------------- */

static int sensor_read_value(u16 *value)
{
    int high;
    int low;

    high =
        i2c_smbus_read_byte_data(sensor_client,
                                 SENSOR_REG_DATA_HIGH);

    if (high < 0) {
        return high;
    }

    low =
        i2c_smbus_read_byte_data(sensor_client,
                                 SENSOR_REG_DATA_LOW);

    if (low < 0) {
        return low;
    }

    *value =
        ((high & 0xFF) << 8) |
        (low & 0xFF);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Open                                                                      */
/* ------------------------------------------------------------------------- */

static int sensor_open(struct inode *inode,
                       struct file *file)
{
    pr_info("%s: opened\n",
            SENSOR_DRIVER_NAME);

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
    u16 sensor_value;

    int ret;

    if (count < sizeof(sensor_value)) {
        return -EINVAL;
    }

    mutex_lock(&sensor_mutex);

    ret = sensor_read_value(&sensor_value);

    mutex_unlock(&sensor_mutex);

    if (ret) {
        return ret;
    }

    if (copy_to_user(buffer,
                     &sensor_value,
                     sizeof(sensor_value))) {

        return -EFAULT;
    }

    pr_info("%s: sensor value = %u\n",
            SENSOR_DRIVER_NAME,
            sensor_value);

    return sizeof(sensor_value);
}


/* ------------------------------------------------------------------------- */
/* Write                                                                     */
/* ------------------------------------------------------------------------- */

static ssize_t sensor_write(struct file *file,
                            const char __user *buffer,
                            size_t count,
                            loff_t *offset)
{
    struct sensor_write_data data;

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

    ret = sensor_write_reg(data.reg,
                           data.value);

    mutex_unlock(&sensor_mutex);

    if (ret < 0) {
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
            SENSOR_DRIVER_NAME);

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

static int sensor_probe(struct i2c_client *client)
{
    int ret;

    sensor_client = client;

    pr_info("%s: probing I2C address 0x%02X\n",
            SENSOR_DRIVER_NAME,
            client->addr);

    /*
     * Verify sensor identity.
     */
    ret =
        i2c_smbus_read_byte_data(client,
                                 SENSOR_REG_WHO_AM_I);

    if (ret < 0) {

        pr_err("%s: sensor communication failed\n",
               SENSOR_DRIVER_NAME);

        return ret;
    }

    pr_info("%s: WHO_AM_I = 0x%02X\n",
            SENSOR_DRIVER_NAME,
            ret);

    /*
     * Create character device.
     */
    ret = alloc_chrdev_region(&sensor_dev,
                              0,
                              1,
                              SENSOR_DRIVER_NAME);

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
        class_create(SENSOR_CLASS_NAME);

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
                      SENSOR_DEVICE_NAME);

    if (IS_ERR(sensor_device)) {

        ret = PTR_ERR(sensor_device);

        class_destroy(sensor_class);

        cdev_del(&sensor_cdev);

        unregister_chrdev_region(sensor_dev,
                                 1);

        return ret;
    }

    /*
     * Example sensor initialization.
     */
    sensor_write_reg(SENSOR_REG_CONFIG,
                     SENSOR_CONFIG_DEFAULT);

    pr_info("%s: created /dev/%s\n",
            SENSOR_DRIVER_NAME,
            SENSOR_DEVICE_NAME);

    return 0;
}


/* ------------------------------------------------------------------------- */
/* Remove                                                                    */
/* ------------------------------------------------------------------------- */

static void sensor_remove(struct i2c_client *client)
{
    device_destroy(sensor_class,
                   sensor_dev);

    class_destroy(sensor_class);

    cdev_del(&sensor_cdev);

    unregister_chrdev_region(sensor_dev,
                             1);

    sensor_client = NULL;

    pr_info("%s: removed\n",
            SENSOR_DRIVER_NAME);
}


/* ------------------------------------------------------------------------- */
/* Device ID                                                                 */
/* ------------------------------------------------------------------------- */

static const struct i2c_device_id sensor_id[] = {
    {
        SENSOR_I2C_DEVICE_NAME,
        0
    },
    { }
};

MODULE_DEVICE_TABLE(i2c, sensor_id);


/* ------------------------------------------------------------------------- */
/* I2C Driver                                                                */
/* ------------------------------------------------------------------------- */

static struct i2c_driver sensor_driver = {
    .driver = {
        .name = SENSOR_DRIVER_NAME,
    },

    .probe = sensor_probe,

    .remove = sensor_remove,

    .id_table = sensor_id,
};


/* ------------------------------------------------------------------------- */
/* Module                                                                    */
/* ------------------------------------------------------------------------- */

module_i2c_driver(sensor_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Embedded Software Engineer");
MODULE_DESCRIPTION("BeagleBone Black I2C Sensor Driver");
MODULE_VERSION("1.0");
