#include "alsa_output.h"
#include "drum_device.h"
#include <stdio.h>
#include <stdlib.h>

static rimshot_drum_device_note_t default_notes[] = {
	{ 9, 38 }, /* acoustic snare */
   	{ 9, 47 }, /* low-mid tom */
    { 9, 45 }, /* low tom */
	{ 9, 43 }, /* high floor tom */
	{ 9, 42 }, /* closed hi-hat */
	{ 9, 49 }, /* crash cymbal */
	{ 9, 35 }  /* acoustic bass drum */
};

static rimshot_drum_device_key_bits_t snare_bits[] = {
	{ 0, 0x04, 0x04 },
	{ 1, 0x04, 0x04 },
	{ 0, 0, 0 }
};

static rimshot_drum_device_key_bits_t high_tom_bits[] = {
	{ 9, 0xff, 0xff },
	{ 2, 0x08, 0x00 },
	{ 0, 0x08, 0x08 },
	{ 1, 0x08, 0x08 },
	{ 0, 0, 0 }
};

static rimshot_drum_device_key_bits_t low_tom_bits[] = {
	{ 1, 0x04, 0x04 },
	{ 0, 0x01, 0x01 },
	{ 0, 0, 0 }
};

static rimshot_drum_device_key_bits_t floor_tom_bits[] = {
	{ 1, 0x08, 0x08 },
	{ 0, 0x02, 0x02 },
	{ 0, 0, 0 }
};

static rimshot_drum_device_key_bits_t hi_hat_bits[] = {
	{ 1, 0x04, 0x04 },
	{ 0, 0x08, 0x08 },
	{ 0, 0, 0 }
};

static rimshot_drum_device_key_bits_t crash_cymbal_bits[] = {
	{ 1, 0x04, 0x04 },
	{ 0, 0x02, 0x02 },
	{ 0, 0, 0 }
};

static rimshot_drum_device_key_bits_t base_bits[] = {
	{ 0, 0x10, 0x10 },
	{ 0, 0, 0 }
};

static rimshot_drum_device_key_t ps3_keys[] = {
	{ RIMSHOT_DRUM_DEVICE_KEY_TYPE_SNARE,        12, snare_bits        },
	{ RIMSHOT_DRUM_DEVICE_KEY_TYPE_HIGH_TOM,     11, high_tom_bits     },
	{ RIMSHOT_DRUM_DEVICE_KEY_TYPE_LOW_TOM,      14, low_tom_bits      },
	{ RIMSHOT_DRUM_DEVICE_KEY_TYPE_FLOOR_TOM,    13, floor_tom_bits    },
	{ RIMSHOT_DRUM_DEVICE_KEY_TYPE_HI_HAT,       11, hi_hat_bits       },
	{ RIMSHOT_DRUM_DEVICE_KEY_TYPE_CRASH_CYMBAL, 13, crash_cymbal_bits },
	{ RIMSHOT_DRUM_DEVICE_KEY_TYPE_BASE,         15, base_bits         },
	{ 0, 0, NULL }
};

static void
loop (rimshot_drum_device_t *dev, rimshot_alsa_output_t *output)
{
	/* only 3 keys can be activated at once */
	rimshot_drum_device_keys_pressed_t pressed[3];
	int num_pressed;

	while (1) {
		num_pressed = rimshot_drum_device_get_keys_pressed (dev, pressed);

		if (num_pressed > 0) {
			int i;

			printf ("pressed %i keys", num_pressed);

			for (i = 0; i < num_pressed; i++) {
				rimshot_drum_device_note_t *note = rimshot_drum_device_get_note (dev, pressed[i].type);
				rimshot_alsa_output_play_note (output, note->channel, note->note, pressed[i].velocity);

				printf (" - %i(%u)", pressed[i].type, pressed[i].velocity);
			}

			printf ("\n");
		}
	}
}

int
main (int argc, char **argv)
{
	rimshot_drum_device_t *dev;
	rimshot_alsa_output_t *output;

	output = rimshot_alsa_output_open ("default", 128, 0);
	if (!output) {
		fprintf (stderr, "failed to open output device\n");
		exit (-1);
	}

	dev = rimshot_drum_device_open (default_notes, ps3_keys);
	if (!dev) {
		fprintf (stderr, "failed to open drum device\n");
		exit (-1);
	}

	loop (dev, output);

	return 0;
}
