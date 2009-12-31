#ifndef __RIMSHOT_DRUM_DEVICE_H__
#define __RIMSHOT_DRUM_DEVICE_H__

/* this might very well be device-specific and should be part of known_devices */
#define RIMSHOT_DRUM_DEVICE_MAX_PACKET_LEN 0x40
#define RIMSHOT_DRUM_DEVICE_INTERRUPT_PACKET_LEN 0x13
#define RIMSHOT_DRUM_DEVICE_INTERRUPT_IN_ENDPOINT 1

#define RIMSHOT_DRUM_DEVICE_READ_TIMEOUT 10000

typedef enum {
	RIMSHOT_DRUM_DEVICE_KEY_TYPE_SNARE = 0,
	RIMSHOT_DRUM_DEVICE_KEY_TYPE_HIGH_TOM,
	RIMSHOT_DRUM_DEVICE_KEY_TYPE_LOW_TOM,
	RIMSHOT_DRUM_DEVICE_KEY_TYPE_FLOOR_TOM,
	RIMSHOT_DRUM_DEVICE_KEY_TYPE_HI_HAT,
	RIMSHOT_DRUM_DEVICE_KEY_TYPE_CRASH_CYMBAL,
	RIMSHOT_DRUM_DEVICE_KEY_TYPE_BASE
} rimshot_drum_device_keys_type_t;

typedef struct rimshot_drum_device_keys_pressed_St {
	rimshot_drum_device_keys_type_t type;
	unsigned int velocity;
} rimshot_drum_device_keys_pressed_t;

typedef struct rimshot_drum_device_note_St {
   int channel;
   int note;
} rimshot_drum_device_note_t;

typedef struct rimshot_drum_device_key_bits_St {
	int byte;
	int mask;
	int value;
} rimshot_drum_device_key_bits_t;

typedef struct rimshot_drum_device_key_St {
	rimshot_drum_device_keys_type_t type;
	int velocity_byte;
	rimshot_drum_device_key_bits_t *key_bits;
} rimshot_drum_device_key_t;

typedef struct rimshot_drum_device_St rimshot_drum_device_t;

rimshot_drum_device_t *rimshot_drum_device_open (rimshot_drum_device_note_t *notes, rimshot_drum_device_key_t *keys);
int rimshot_drum_device_get_keys_pressed (rimshot_drum_device_t *dev, rimshot_drum_device_keys_pressed_t *keys);
rimshot_drum_device_note_t *rimshot_drum_device_get_note (rimshot_drum_device_t *dev, rimshot_drum_device_keys_type_t type);

#endif
