#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/usb.h>

static struct usb_device_id id_table[] = {
	{ USB_DEVICE(0x12ba, 0x0210) },
	{ }
};

MODULE_DEVICE_TABLE (usb, id_table);

static int
drum_rocker_probe (struct usb_interface *interface, const struct usb_device_id *id)
{
	dev_info (&interface->dev, "drum rocker device now attached\n");
	return 0;
}

static void
drum_rocker_disconnect (struct usb_interface *interface)
{
	dev_info (&interface->dev, "drum rocker now disconnected\n");
}

static struct usb_driver drum_rocker_driver = {
	.name       = "drumdocker",
	.probe      = drum_rocker_probe,
	.disconnect = drum_rocker_disconnect,
	.id_table   = id_table
};

static int
__init drum_rocker_init ()
{
	int ret = 0;

	ret = usb_register (&drum_rocker_driver);
	if (ret) {
		err ("usb_register failed. Error %d", ret);
	}

	return ret;
}

static void
__exit drum_rocker_exit ()
{
	usb_deregister (&drum_rocker_driver);
}

module_init (drum_rocker_init);
module_exit (drum_rocker_exit);

MODULE_AUTHOR("Florian Ragwitz <rafl@debian.org>");
MODULE_DESCRIPTION("Ion Drum Rocker driver");
MODULE_LICENSE("GPL");
