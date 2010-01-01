#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/usb.h>

#define DRUM_ROCKER_BUFSIZE 64

struct drum_rocker {
	char inbuf[DRUM_ROCKER_BUFSIZE];
};

static struct usb_device_id id_table[] = {
	{ USB_DEVICE(0x12ba, 0x0210) },
	{ }
};

MODULE_DEVICE_TABLE (usb, id_table);

static void
irq_in (struct urb *urb)
{
	struct drum_rocker *drum = urb->context;

	switch (urb->status) {
		case 0: /* success */
			printk (KERN_INFO "foo %d\n", urb->actual_length);
			usb_submit_urb (urb, GFP_ATOMIC);
			break;
		default:
			dev_warn (&urb->dev->dev, "input irq status %d received\n", urb->status);
	}

}

static int
drum_rocker_probe (struct usb_interface *interface, const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev (interface);
	int pipe = usb_rcvintpipe (dev, 1);
	struct urb *urbin = usb_alloc_urb (0, GFP_KERNEL);
	struct drum_rocker *drum;

	drum = kmalloc (sizeof (struct drum_rocker), GFP_KERNEL);
	memset (drum, 0x00, sizeof (*drum));
	usb_set_intfdata (interface, drum);

	dev_info (&interface->dev, "drum rocker device attached\n");

	usb_fill_int_urb (urbin, dev, pipe, drum->inbuf, DRUM_ROCKER_BUFSIZE, irq_in, drum, 10);
	usb_submit_urb (urbin, GFP_ATOMIC);

	return 0;
}

static void
drum_rocker_disconnect (struct usb_interface *interface)
{
	dev_info (&interface->dev, "drum rocker disconnected\n");
}

static struct usb_driver drum_rocker_driver = {
	.name       = "drum_rocker",
	.probe      = drum_rocker_probe,
	.disconnect = drum_rocker_disconnect,
	.id_table   = id_table
};

static int
__init drum_rocker_init (void)
{
	int ret = 0;

	ret = usb_register (&drum_rocker_driver);
	if (ret) {
		err ("usb_register failed. Error %d", ret);
	}

	return ret;
}

static void
__exit drum_rocker_exit (void)
{
	usb_deregister (&drum_rocker_driver);
}

module_init (drum_rocker_init);
module_exit (drum_rocker_exit);

MODULE_AUTHOR("Florian Ragwitz <rafl@debian.org>");
MODULE_DESCRIPTION("Ion Drum Rocker driver");
MODULE_LICENSE("GPL");
