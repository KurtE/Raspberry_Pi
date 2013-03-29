
/*
* This extra small demo sends a random samples to your speakers.
*/
#include <alsa/asoundlib.h>
#include <math.h>
static char *device = "default"; /* playback device */
snd_output_t *output = NULL;
unsigned char buffer[24*1024]; /* some random data */
#define sampleRate 48000L
extern int FillSoundBuffer(unsigned char *psz, int cb, unsigned int freq, unsigned long duration);

int main(void)
{
	int err;
	unsigned int i;
	int cb;
	snd_pcm_t *handle;
	snd_pcm_sframes_t frames;
	int freq = 500;
	for (i = 0; i < sizeof(buffer); i++)
		buffer[i] = random() & 0xff;

	if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	if ((err = snd_pcm_set_params(handle,
			SND_PCM_FORMAT_U8,
			SND_PCM_ACCESS_RW_INTERLEAVED,
			1,
			48000,
			1,
			500000)) < 0) { /* 0.5sec */
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	for (i = 0; i < 16; i++) {
		cb = FillSoundBuffer(buffer, sizeof(buffer), freq, 500);
		frames = snd_pcm_writei(handle, buffer, cb);
		printf ("freq %i frames: %li\n", freq, frames);
		if (frames < 0)
			frames = snd_pcm_recover(handle, frames, 0);
		if (frames < 0) {
			printf("snd_pcm_writei failed: %s\n", snd_strerror(err));
			break;
		}
		if (frames > 0 && frames < (long)sizeof(buffer))
			printf("Short write (expected %li, wrote %li)\n", (long)sizeof(buffer), frames);
		freq += 250;
	}
	snd_pcm_close(handle);
	return 0;
}
int FillSoundBuffer(unsigned char *psz, int cb, unsigned int freq, unsigned long duration) {
	double sample;
	unsigned char	val;
	int	numSamples = ((long)((long)duration * sampleRate))/1000L;	// 
	printf("D: %lu RT: %lu, Samples %u\n", duration, sampleRate, numSamples);

	for (int i = 0; i < numSamples; ++i) {
		sample = sin(2 * M_PI * i / (sampleRate/freq));
		val = (unsigned char)(sample * 255);
		*psz++ = val;
	}
	return numSamples;
}
