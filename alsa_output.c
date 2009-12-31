#include "alsa_output.h"
#include <alsa/asoundlib.h>

struct rimshot_alsa_output_St {
	snd_seq_t *seq;
	int port;
};

rimshot_alsa_output_t *
rimshot_alsa_output_open (const char *name, int client, int port)
{
	snd_seq_t *seq;
	rimshot_alsa_output_t *alsa;

	if (snd_seq_open (&seq, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0) {
		return NULL;
	}

	snd_seq_set_client_name (seq, "drumkit");

	alsa = (rimshot_alsa_output_t *)calloc (1, sizeof (rimshot_alsa_output_t));
	alsa->seq = seq;
	alsa->port = snd_seq_create_simple_port (seq, "drumkit output",
	                                         SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
	                                         SND_SEQ_PORT_TYPE_APPLICATION);

	snd_seq_connect_to (seq, alsa->port, client, port);

	return alsa;
}

void
rimshot_alsa_output_play_note (rimshot_alsa_output_t *output, int channel, int note, unsigned int velocity)
{
	snd_seq_event_t ev;

	snd_seq_ev_clear (&ev);
	snd_seq_ev_set_source (&ev, output->port);
	snd_seq_ev_set_subs (&ev);
	snd_seq_ev_set_direct (&ev);

	snd_seq_ev_set_noteon (&ev, channel, note, velocity);
	snd_seq_event_output_direct (output->seq, &ev);
}
