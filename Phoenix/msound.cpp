//=============================================================================
//Project Lynxmotion Phoenix
//
// Sound support - A few different options...
//
//NEW IN V2.X
//=============================================================================
//
//KNOWN BUGS:
//    - Lots ;)
//
//=============================================================================
// Header Files
//=============================================================================

#define DEFINE_HEX_GLOBALS

#include "Hex_Cfg.h"
#include "Phoenix.h"


//==============================================================================
// First version for support of simiply using a simple speaker...
//==============================================================================
//==============================================================================
//    SoundNoTimer - Quick and dirty tone function to try to output a frequency
//            to a speaker for some simple sounds.
//==============================================================================
#ifdef SOUND_PIN
void SoundNoTimer(unsigned long duration,  unsigned int frequency)
{
#ifdef __AVR__
  volatile uint8_t *pin_port;
  volatile uint8_t pin_mask;
#else
  volatile uint32_t *pin_port;
  volatile uint16_t pin_mask;
#endif
  long toggle_count = 0;
  long lusDelayPerHalfCycle;

  // Set the pinMode as OUTPUT
  pinMode(SOUND_PIN, OUTPUT);

  pin_port = portOutputRegister(digitalPinToPort(SOUND_PIN));
  pin_mask = digitalPinToBitMask(SOUND_PIN);

  toggle_count = 2 * frequency * duration / 1000;
  lusDelayPerHalfCycle = 1000000L/(frequency * 2);

  // if we are using an 8 bit timer, scan through prescalars to find the best fit
  while (toggle_count--) {
    // toggle the pin
    *pin_port ^= pin_mask;

    // delay a half cycle
    delayMicroseconds(lusDelayPerHalfCycle);
  }    
  *pin_port &= ~(pin_mask);  // keep pin low after stop

}

void MSound(byte cNotes, ...)
{
  va_list ap;
  unsigned int uDur;
  unsigned int uFreq;
  va_start(ap, cNotes);

  while (cNotes > 0) {
    uDur = va_arg(ap, unsigned int);
    uFreq = va_arg(ap, unsigned int);
    SoundNoTimer(uDur, uFreq);
    cNotes--;
  }
  va_end(ap);
}
//==============================================================================
// Use PCM sound on systems that support it.
//==============================================================================
#elif defined(OPT_PCMSOUND)
#include <alsa/asoundlib.h>
#include <math.h>
static char *device = "default"; /* playback device */
snd_output_t *output = NULL;
unsigned char buffer[24*1024]; /* some random data */
#define sampleRate 48000L


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

void MSound(byte cNotes, ...)
{
	int err;
	int cb;
	snd_pcm_t *handle;
	snd_pcm_sframes_t frames;

	va_list ap;
	unsigned int uDur;
	unsigned int uFreq;
	va_start(ap, cNotes);

	if (cNotes) {
		// Open up the PCM...
		if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
			printf("Playback open error: %s\n", snd_strerror(err));
			return;
		}
	
		while (cNotes > 0) {
			uDur = va_arg(ap, unsigned int);
			uFreq = va_arg(ap, unsigned int);
			
			// Now we need to set the paramters for this sound...
			if ((err = snd_pcm_set_params(handle,
					SND_PCM_FORMAT_U8,
					SND_PCM_ACCESS_RW_INTERLEAVED,
					1,
					48000,
					1,
					uDur*1000)) < 0) { /* convert ms to us */
				printf("Playback open error: %s\n", snd_strerror(err));
				break;
			}	
			

			cb = FillSoundBuffer(buffer, sizeof(buffer), uFreq , uDur);
			frames = snd_pcm_writei(handle, buffer, cb);
			printf ("freq %i frames: %li\n", uFreq , frames);
			if (frames < 0)
				frames = snd_pcm_recover(handle, frames, 0);
			if (frames < 0) {
				printf("snd_pcm_writei failed: %s\n", snd_strerror(err));
				break;
			}
			if (frames > 0 && frames < (long)sizeof(buffer)) {
				printf("Short write (expected %li, wrote %li)\n", (long)sizeof(buffer), frames);
			}	
			cNotes--;
			snd_pcm_drain(handle);
		}

		snd_pcm_close(handle);
	}
	va_end(ap);
}


#else
//==============================================================================
// else no sound support
//==============================================================================
void MSound(byte cNotes, ...)
{
}
#endif
