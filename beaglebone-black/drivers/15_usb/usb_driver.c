/*
 * BeagleBone Black - USB Driver Demo
 *
 * File:
 *     usb_driver.c
 *
 * Purpose:
 *     Demonstrates a Linux USB device driver using the USB core.
 *
 * Flow:
 *
 *     USB Device
 *          |
 *          v
 *     USB Host Controller
 *          |
 *          v
 *       USB Core
 *          |
 *          v
 *     Device Matching
 *          |
 *          v
 *       probe()
 *          |
 *          v
 *     USB Interface
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/slab.h>

#include "usb_driver.h"


/* ------------------------------------------------------------------------- */
/* Driver Private Data                                                       */
/* ------------------------------------------------------------------------- */

struct bbb_usb_device {
	struct usb_device *udev;
	struct usb_interface *interface;

	unsigned int bulk_in_size;
	unsigned int bulk_out_size;

	unsigned char bulk_in_endpoint;
	unsigned char bulk_out_endpoint;
};


/* ------------------------------------------------------------------------- */
/* USB Device ID Table                                                       */
/* ------------------------------------------------------------------------- */

/*
 * Example USB device.
 *
 * Replace these IDs with the VID/PID of the actual USB device
 * being tested.
 *
 * Example:
 *
 *     Vendor ID  = 0x1234
 *     Product ID = 0x5678
 */
static const struct usb_device_id bbb_usb_id_table[] = {

	{
		USB_DEVICE(
			USB_VENDOR_ID_TEST,
			USB_PRODUCT_ID_TEST
		)
	},

	{ }
};

MODULE_DEVICE_TABLE(usb, bbb_usb_id_table);


/* ------------------------------------------------------------------------- */
/* Find USB Endpoints                                                        */
/* ------------------------------------------------------------------------- */

static int bbb_usb_find_endpoints(
		struct bbb_usb_device *dev)
{
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;

	int i;


	iface_desc =
		dev->interface->cur_altsetting;


	for (i = 0;
	     i < iface_desc->desc.bNumEndpoints;
	     i++) {

		endpoint =
			&iface_desc->endpoint[i].desc;


		/*
		 * Bulk IN endpoint.
		 */
		if (usb_endpoint_is_bulk_in(endpoint)) {

			dev->bulk_in_endpoint =
				endpoint->bEndpointAddress;

			dev->bulk_in_size =
				usb_endpoint_maxp(endpoint);

			dev_info(
				&dev->interface->dev,
				"Bulk IN endpoint: 0x%02X\n",
				dev->bulk_in_endpoint);

			dev_info(
				&dev->interface->dev,
				"Bulk IN max packet: %u\n",
				dev->bulk_in_size);
		}


		/*
		 * Bulk OUT endpoint.
		 */
		if (usb_endpoint_is_bulk_out(endpoint)) {

			dev->bulk_out_endpoint =
				endpoint->bEndpointAddress;

			dev->bulk_out_size =
				usb_endpoint_maxp(endpoint);

			dev_info(
				&dev->interface->dev,
				"Bulk OUT endpoint: 0x%02X\n",
				dev->bulk_out_endpoint);

			dev_info(
				&dev->interface->dev,
				"Bulk OUT max packet: %u\n",
				dev->bulk_out_size);
		}
	}


	return 0;
}


/* ------------------------------------------------------------------------- */
/* USB Probe                                                                 */
/* ------------------------------------------------------------------------- */

static int bbb_usb_probe(
		struct usb_interface *interface,
		const struct usb_device_id *id)
{
	struct bbb_usb_device *dev;

	struct usb_device *udev;

	int ret;


	/*
	 * Get USB device.
	 */
	udev =
		interface_to_usbdev(interface);


	dev_info(&interface->dev,
		 "BBB USB driver probe\n");


	dev_info(&interface->dev,
		 "Vendor ID : 0x%04X\n",
		 le16_to_cpu(udev->descriptor.idVendor));


	dev_info(&interface->dev,
		 "Product ID: 0x%04X\n",
		 le16_to_cpu(udev->descriptor.idProduct));


	dev_info(&interface->dev,
		 "Bus       : %03d\n",
		 udev->bus->busnum);


	dev_info(&interface->dev,
		 "Device    : %03d\n",
		 udev->devnum);


	/*
	 * Allocate driver data.
	 */
	dev =
		kzalloc(sizeof(*dev),
			GFP_KERNEL);

	if (!dev)
		return -ENOMEM;


	dev->udev =
		usb_get_dev(udev);

	dev->interface =
		interface;


	/*
	 * Find USB endpoints.
	 */
	ret =
		bbb_usb_find_endpoints(dev);

	if (ret) {

		dev_err(&interface->dev,
			"Failed to find USB endpoints\n");

		usb_put_dev(dev->udev);

		kfree(dev);

		return ret;
	}


	/*
	 * Store driver private data.
	 */
	usb_set_intfdata(interface,
			 dev);


	dev_info(&interface->dev,
		 "USB device successfully attached\n");


	return 0;
}


/* ------------------------------------------------------------------------- */
/* USB Disconnect                                                            */
/* ------------------------------------------------------------------------- */

static void bbb_usb_disconnect(
		struct usb_interface *interface)
{
	struct bbb_usb_device *dev;


	dev =
		usb_get_intfdata(interface);


	usb_set_intfdata(interface,
			 NULL);


	if (!dev)
		return;


	dev_info(&interface->dev,
		 "USB device disconnected\n");


	/*
	 * Release USB device reference.
	 */
	usb_put_dev(dev->udev);


	/*
	 * Free driver data.
	 */
	kfree(dev);
}


/* ------------------------------------------------------------------------- */
/* USB Driver                                                                */
/* ------------------------------------------------------------------------- */

static struct usb_driver bbb_usb_driver = {

	.name =
		USB_DRIVER_NAME,

	.id_table =
		bbb_usb_id_table,

	.probe =
		bbb_usb_probe,

	.disconnect =
		bbb_usb_disconnect,
};


/* ------------------------------------------------------------------------- */
/* Module Init                                                               */
/* ------------------------------------------------------------------------- */

static int __init bbb_usb_init(void)
{
	int ret;


	pr_info("BBB USB driver initializing\n");


	ret =
		usb_register(&bbb_usb_driver);

	if (ret) {

		pr_err(
			"Failed to register USB driver: %d\n",
			ret);

		return ret;
	}


	pr_info(
		"BBB USB driver registered successfully\n");


	return 0;
}


/* ------------------------------------------------------------------------- */
/* Module Exit                                                               */
/* ------------------------------------------------------------------------- */

static void __exit bbb_usb_exit(void)
{
	pr_info("BBB USB driver unloading\n");


	usb_deregister(
		&bbb_usb_driver);


	pr_info(
		"BBB USB driver unloaded\n");
}


module_init(bbb_usb_init);
module_exit(bbb_usb_exit);


/* ------------------------------------------------------------------------- */
/* Module Information                                                        */
/* ------------------------------------------------------------------------- */

MODULE_LICENSE("GPL");

MODULE_AUTHOR(
	"Embedded Software Engineer");

MODULE_DESCRIPTION(
	"BeagleBone Black Linux USB Driver Demo");

MODULE_VERSION("1.0");
