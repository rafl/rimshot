#include "drum_device.h"
#include <sys/time.h>
#include <string.h>
#include <usb.h>

typedef struct usb_dev_id_St {
	uint16_t vendor_id;
	uint16_t product_id;
} usb_dev_id_t;

struct rimshot_drum_device_St {
	struct usb_device *usb_dev;
	usb_dev_handle *usb_handle;
	int base_down;
	rimshot_drum_device_key_t *keys;
	rimshot_drum_device_note_t *notes;
	/* we don't need timing information for the base drum. it really just fires once */
	struct timeval last_hit[RIMSHOT_DRUM_DEVICE_KEY_TYPE_BASE];
	char last_buf[RIMSHOT_DRUM_DEVICE_MAX_PACKET_LEN];
};

static usb_dev_id_t known_devices[] = {
	{ 0x12ba, 0x0210 }, /* Licensed by Sony Computer Entertainment America Harmonix Drum Kit for PlayStation(R)3 */
};

/*
   00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
      |||||     ||       |                                                          ||||||||          |||||||| |||||||| |||||||| |||||||| ||||||||
      |||||     ||       |                                                          ||||||||          |||||||| |||||||| |||||||| |||||||| `+++++++------------------------------ all bits set on basedrum down
      |||||     ||       |                                                          ||||||||          |||||||| |||||||| |||||||| ||||||||
      |||||     ||       |                                                          ||||||||          |||||||| |||||||| |||||||| `+++++++--------------------------------------- low tom velocity
      |||||     ||       |                                                          ||||||||          |||||||| |||||||| ||||||||
      |||||     ||       |                                                          ||||||||          |||||||| |||||||| `+++++++------------------------------------------------ floor tom and crash cymbal velocity
      |||||     ||       |                                                          ||||||||          |||||||| ||||||||
      |||||     ||       |                                                          ||||||||          |||||||| `+++++++--------------------------------------------------------- snare drum velocity (soft -> high (0xffffffXX), hard -> low (0x22))
      |||||     ||       |                                                          ||||||||          ||||||||
      |||||     ||       |                                                          ||||||||          `+++++++------------------------------------------------------------------ high tom and hi-hat (no way to get vel. for both when played simult.?) velocity (same values as snare)
      |||||     ||       |                                                          ||||||||
      |||||     ||       |                                                          `+++++++------------------------------------------------------------------------------------ on for high tom
      |||||     ||       |
      |||||     ||       `------------------------------------------------------------------------------------------------------------------------------------------------------ off for high tom, on otherwise
      |||||     ||
      |||||     |`-------------------------------------------------------------------------------------------------------------------------------------------------------------- on for snare, hi-hat, and crash cymbal
      |||||     |
      |||||     `--------------------------------------------------------------------------------------------------------------------------------------------------------------- on for high tom, low tom, and floor tom
      |||||
      ||||`--------------------------------------------------------------------------------------------------------------------------------------------------------------------- on for low tom
      ||||
      |||`---------------------------------------------------------------------------------------------------------------------------------------------------------------------- on for floor tom and crash cymbal
      |||
      ||`----------------------------------------------------------------------------------------------------------------------------------------------------------------------- on for snare
      ||
      |`------------------------------------------------------------------------------------------------------------------------------------------------------------------------ on for high tom and hi-hat
      |
      `------------------------------------------------------------------------------------------------------------------------------------------------------------------------- on when basedrum down
*/

static int
device_is_known (struct usb_device *dev)
{
	unsigned int i;
	for (i = 0; i < (sizeof (known_devices) / sizeof (known_devices[0])); i++) {
		usb_dev_id_t id = known_devices[i];

		if (dev->descriptor.idVendor == id.vendor_id
		 && dev->descriptor.idProduct == id.product_id) {
			return 1;
		}
	}

	return 0;
}

static struct usb_device *
device_find ()
{
	struct usb_bus *bus;

	usb_init ();
	usb_find_busses ();

	if (usb_find_devices () <= 0) {
		return NULL;
	}

	for (bus = usb_busses; bus; bus = bus->next) {
		struct usb_device *dev;

		for (dev = bus->devices; dev; dev = dev->next) {
			if (device_is_known (dev)) {
				return dev;
			}
		}
	}

	return NULL;
}

rimshot_drum_device_t *
rimshot_drum_device_open (rimshot_drum_device_note_t *notes, rimshot_drum_device_key_t *keys)
{
	struct usb_device *usb_dev;
	usb_dev_handle *handle;
	rimshot_drum_device_t *dev;

	usb_dev = device_find ();
	if (!usb_dev) {
		return NULL;
	}

	handle = usb_open (usb_dev);
	if (!handle) {
		return NULL;
	}

	dev = (rimshot_drum_device_t *)calloc (1, sizeof (rimshot_drum_device_t));
	dev->usb_dev = usb_dev;
	dev->usb_handle = handle;
	dev->keys = keys;
	dev->notes = notes;

	return dev;
}

static void
diff_timeval (struct timeval *result, struct timeval *y, struct timeval *x)
{
	if (x->tv_usec < y->tv_usec) {
		int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
		y->tv_usec -= 1000000 * nsec;
		y->tv_sec += nsec;
	}

	if (x->tv_usec - y->tv_usec > 1000000) {
		int nsec = (x->tv_usec - y->tv_usec) / 1000000;
		y->tv_usec += 1000000 * nsec;
		y->tv_sec -= nsec;
	}

	result->tv_sec = x->tv_sec - y->tv_sec;
	result->tv_usec = x->tv_usec - y->tv_usec;
}

static int
is_key_pressed (rimshot_drum_device_t *dev, rimshot_drum_device_keys_type_t key_type, const char *buf)
{
	int i = 0, matched_bits = 0;
	struct timeval tv, diff;
	rimshot_drum_device_key_t key;
	rimshot_drum_device_key_bits_t bits;

	key = dev->keys[key_type];

	while (bits = key.key_bits[i], bits.mask != 0) {
		if ((buf[bits.byte] & bits.mask) == bits.value) {
			matched_bits++;
		}
		i++;
	}

	if (matched_bits != i) {
		return 0;
	}

	if (key_type == RIMSHOT_DRUM_DEVICE_KEY_TYPE_BASE) {
		return 1;
	}

	gettimeofday (&tv, NULL);
	diff_timeval (&diff, &dev->last_hit[key_type], &tv);
	if (diff.tv_sec < 1 && diff.tv_usec < 50000) {
		return 0;
	}

	dev->last_hit[key_type].tv_sec = tv.tv_sec;
	dev->last_hit[key_type].tv_usec = tv.tv_usec;

	return 1;
}

static int
keys_pressed (rimshot_drum_device_t *dev, const char *buf, rimshot_drum_device_keys_pressed_t *keys)
{
	int i = 0, matched_keys = 0;
	rimshot_drum_device_key_t key;

	while (key = dev->keys[i], key.key_bits != NULL) {
		if (is_key_pressed (dev, key.type, buf)) {
			unsigned int velocity;

			if (key.type == RIMSHOT_DRUM_DEVICE_KEY_TYPE_BASE) {
				if (dev->base_down) {
					i++;
					continue;
				}

				dev->base_down = 1;
			}

			velocity = key.type == RIMSHOT_DRUM_DEVICE_KEY_TYPE_BASE
			               ? 0xffffffff
			               : 0xffffffff - buf[key.velocity_byte];

			if (velocity > 50) {
				velocity = 50;
			}

			keys[matched_keys].velocity = velocity + 50;
			keys[matched_keys].type = key.type;

			matched_keys++;
		}

		i++;
	}

	return matched_keys;
}

static int
read_state (rimshot_drum_device_t *dev, char *buf, size_t buflen)
{
	int read;

	read = usb_interrupt_read (dev->usb_handle,
	                           RIMSHOT_DRUM_DEVICE_INTERRUPT_IN_ENDPOINT,
	                           (char *)buf, buflen,
	                           RIMSHOT_DRUM_DEVICE_READ_TIMEOUT);

	if (read != RIMSHOT_DRUM_DEVICE_INTERRUPT_PACKET_LEN) {
		return -1;
	}

	return read;
}

int
rimshot_drum_device_get_keys_pressed (rimshot_drum_device_t *dev, rimshot_drum_device_keys_pressed_t *keys)
{
	char buf[RIMSHOT_DRUM_DEVICE_MAX_PACKET_LEN];
	int read, ret;

	read = read_state (dev, (char *)buf, sizeof (buf));
	if (read < 0) {
		return 0;
	}

	if (memcmp (buf, dev->last_buf, read) == 0) {
		return 0;
	}

	ret = keys_pressed (dev, buf, keys);

	memcpy (dev->last_buf, buf, read);

	if (dev->base_down && !is_key_pressed (dev, RIMSHOT_DRUM_DEVICE_KEY_TYPE_BASE, buf)) {
		dev->base_down = 0;
	}

	return ret;
}

rimshot_drum_device_note_t *
rimshot_drum_device_get_note (rimshot_drum_device_t *dev, rimshot_drum_device_keys_type_t type)
{
	return &dev->notes[type];
}
