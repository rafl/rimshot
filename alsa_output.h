#ifndef __RIMSHOT_ALSA_OUTPUT_H__
#define __RIMSHOT_ALSA_OUTPUT_H__

typedef struct rimshot_alsa_output_St rimshot_alsa_output_t;

rimshot_alsa_output_t *rimshot_alsa_output_open (const char *name, int client, int port);
void rimshot_alsa_output_play_note (rimshot_alsa_output_t *output, int channel, int note, unsigned int velocity);

#endif
