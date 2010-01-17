#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/usb.h>
#include <sound/initval.h>
#include <sound/core.h>
#include <sound/rawmidi.h>

#define DRUM_ROCKER_BUFSIZE 64

struct drum_rocker {
	char inbuf[DRUM_ROCKER_BUFSIZE];
	char prev_inbuf[DRUM_ROCKER_BUFSIZE];
	struct snd_card *card;
};

static struct usb_device_id id_table[] = {
	{ USB_DEVICE(0x12ba, 0x0210) },
	{ }
};

MODULE_DEVICE_TABLE (usb, id_table);

static int index = SNDRV_DEFAULT_IDX1;
static char *id = SNDRV_DEFAULT_STR1;

module_param (index, int, 0444);
MODULE_PARM_DESC (index, "Index value for drum rocker soundcard.");
module_param (id, charp, 0444);
MODULE_PARM_DESC (id, "ID string for drum rocker soundcard.");

static void
irq_in (struct urb *urb)
{
	struct drum_rocker *drum = urb->context;

	switch (urb->status) {
		case 0: /* success */
			if (urb->actual_length != 19) {
				usb_submit_urb (urb, GFP_ATOMIC);
				return;
			}

			if (memcmp (drum->inbuf, drum->prev_inbuf, urb->actual_length) != 0) {
				/* printk (KERN_INFO "foo %d\n", urb->actual_length); */
			}

			memcpy (drum->prev_inbuf, drum->inbuf, urb->actual_length);
			usb_submit_urb (urb, GFP_ATOMIC);

			break;

		default:
			dev_warn (&urb->dev->dev, "input irq status %d received\n", urb->status);
	}

}

static int
drum_rocker_probe (struct usb_interface *interface, const struct usb_device_id *dev_id)
{
	struct usb_device *dev = interface_to_usbdev (interface);
	int pipe = usb_rcvintpipe (dev, 1);
	struct urb *urbin = usb_alloc_urb (0, GFP_KERNEL);
	struct drum_rocker *drum;
	struct snd_card *card;
	struct snd_rawmidi *rmidi;
	int err;

	drum = kmalloc (sizeof (struct drum_rocker), GFP_KERNEL);
	memset (drum, 0x00, sizeof (*drum));
	usb_set_intfdata (interface, drum);

	dev_info (&interface->dev, "drum rocker device attached\n");

	dev_info (&interface->dev, "creating soundcard %i %s", index, id);

	err = snd_card_create (index, id, THIS_MODULE, 0, &card);
	dev_info (&interface->dev, "create soundcard %i", err);
	if (err < 0) {
		return err;
	}

	drum->card = card;

	err = snd_rawmidi_new (card, "Ion Drum Rocker", 0, 1, 0, &rmidi);
	if (err < 0) {
		return err;
	}

	rmidi->info_flags = SNDRV_RAWMIDI_INFO_OUTPUT;

	snd_card_set_dev (card, &dev->dev);

	err = snd_card_register (card);
	if (err < 0) {
		return err;
	}

	usb_fill_int_urb (urbin, dev, pipe, drum->inbuf, DRUM_ROCKER_BUFSIZE, irq_in, drum, 10);
	usb_submit_urb (urbin, GFP_ATOMIC);

	return 0;
}

static void
drum_rocker_disconnect (struct usb_interface *interface)
{
	struct drum_rocker *drum;

	drum = usb_get_intfdata (interface);
	usb_set_intfdata (interface, NULL);
	snd_card_free (drum->card);
	kfree (drum);

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
