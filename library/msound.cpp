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

#include "msound.h"
#include "SDRFunctions.h"

//==============================================================================
// Use PCM sound on systems that support it.
//==============================================================================
#include <alsa/asoundlib.h>
#include <math.h>
static char *device = "default";                  /* playback device */
snd_output_t *output = NULL;
unsigned char buffer[24*1024];                    /* some random data */
#define sampleRate 48000L
static snd_pcm_sframes_t buffer_size;

//#define DEBUG_SOUND
extern int set_hwparams(snd_pcm_t *handle, snd_pcm_hw_params_t *params);
extern int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams);

int FillSoundBuffer(unsigned char *psz, int cb, unsigned int freq, unsigned long duration)
{
    double sample;
    unsigned char   val;
                                                  //
    int numSamples = ((long)((long)duration * sampleRate))/1000L;
#ifdef DEBUG_SOUND
    printf("D: %lu RT: %lu, Samples %u\n", duration, sampleRate, numSamples);
#endif
    for (int i = 0; i < numSamples; ++i)
    {
        sample = sin(2 * M_PI * i / (sampleRate/freq));
        val = (unsigned char)(sample * 255);
        *psz++ = val;
    }
    return numSamples;
}


// Lets try caching out the handle...

snd_pcm_t *g_PCMhandle = 0;
snd_pcm_hw_params_t *g_hwparams = 0;
snd_pcm_sw_params_t *g_swparams;
#define SOUND_RATE 48000
static unsigned int buffer_time = 500000;         /* ring buffer length in us */
static unsigned int period_time = 10000;          /* period time in us */
static snd_pcm_sframes_t period_size;

static unsigned char g_bIDSDRMSound = 0xff;

void MSoundRelease(void)
{
    // No Notes passed in, use this as a signal to close anything that is open...
    if (g_PCMhandle)
    {
        snd_pcm_close(g_PCMhandle);
        g_PCMhandle = 0;                          // clear out the handle
    }
}

void MSound(byte cNotes, ...)
{
    int err;
    int cb;
    snd_pcm_sframes_t frames;

    va_list ap;
    unsigned int uDur;
    unsigned int uFreq;
#ifdef DEBUG_SOUND
    unsigned long ulStart;
#endif
    va_start(ap, cNotes);

    // Open up the PCM...
    if (!g_PCMhandle)
    {
        // See if this is the first time through if so register our release function.
        if (g_bIDSDRMSound == 0xff) 
            g_bIDSDRMSound = SDRRegisterReleaseFunction(&MSoundRelease);
            
        // Tell our quick and dirty Sound registry stuff, that we want to be the current one...    
        SDRSetCurrent(g_bIDSDRMSound);
        
        snd_pcm_hw_params_alloca(&g_hwparams);
        snd_pcm_sw_params_alloca(&g_swparams);
        if ((err = snd_pcm_open(&g_PCMhandle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
        {
#ifdef DEBUG_SOUND
            printf("Playback open error: %s\n", snd_strerror(err));
#endif
            snd_pcm_close(g_PCMhandle);
            g_PCMhandle = 0;                      // clear out the handle
            return;
        }
    
        // Now we need to set the paramters for this sound...
        if ((err = set_hwparams(g_PCMhandle, g_hwparams )) < 0)
        {
#ifdef DEBUG_SOUND
            printf("Setting of hwparams failed: %s\n", snd_strerror(err));
#endif
            return ;
        }
        if ((err = set_swparams(g_PCMhandle, g_swparams)) < 0)
        {
#ifdef DEBUG_SOUND
            printf("Setting of swparams failed: %s\n", snd_strerror(err));
#endif
            snd_pcm_close(g_PCMhandle);
            g_PCMhandle = 0;                      // clear out the handle
            return;
        }
    }
#ifdef DEBUG_SOUND
    printf("C Notes: %i\n", (int)cNotes);
#endif
    while (cNotes > 0)
    {

        uDur = va_arg(ap, unsigned int);
        uFreq = va_arg(ap, unsigned int);
#ifdef WAIT_TILL_NOTE_COMPLETES_BEFORE_NEXT
        // Ok lets set the duration
        snd_pcm_state_t pcmstate;
        int cnt = 500;
        while ((pcmstate = snd_pcm_state(g_PCMhandle)) == SND_PCM_STATE_RUNNING)
        {
            if (!--cnt)
                break;
            delay(1);
        }
#ifdef DEBUG_SOUND
        printf("ST: %i C: %i\n", (int)pcmstate, cnt);
#endif
#endif 
#ifdef DEBUG_SOUND
        ulStart = millis();
#endif
        cb = FillSoundBuffer(buffer, sizeof(buffer), uFreq , uDur);
        unsigned char *pb = buffer;
        while (cb)
        {
            frames = snd_pcm_writei(g_PCMhandle, pb, cb);
#ifdef DEBUG_SOUND
            printf ("freq %i frames: %li T: %li\n", uFreq , frames, millis()-ulStart);
#endif
            if (frames < 0)
            {
                frames = snd_pcm_recover(g_PCMhandle, frames, 1);
#ifdef DEBUG_SOUND
                printf("%i\n", (int)frames);
#endif
            }
            if (frames < 0)
            {
#ifdef DEBUG_SOUND
                printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
#endif
                break;
            }
            if (frames > 0 && frames < cb)
            {
                printf("Short write (expected %i, wrote %li)\n", cb, frames);
            }
            pb += frames;
            cb -= frames;
        }
        cNotes--;
#ifdef DEBUG_SOUND
        printf("T: %li\n", millis()-ulStart);
#endif
    }

    va_end(ap);
#ifdef DEBUG_SOUND
    printf("Return\n");
#endif
}


int set_hwparams(snd_pcm_t *handle, snd_pcm_hw_params_t *params)
{
    unsigned int rrate;
    snd_pcm_uframes_t size;
    int err, dir;
    /* choose all parameters */
    err = snd_pcm_hw_params_any(handle, params);
    if (err < 0)
    {
#ifdef DEBUG_SOUND
        printf("Broken configuration for playback: no configurations available: %s\n", snd_strerror(err));
#endif
        return err;
    }
    /* set hardware resampling */
    err = snd_pcm_hw_params_set_rate_resample(handle, params, 1);
    if (err < 0)
    {
#ifdef DEBUG_SOUND
        printf("Resampling setup failed for playback: %s\n", snd_strerror(err));
#endif
        return err;
    }
    /* set the interleaved read/write format */
    err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0)
    {
#ifdef DEBUG_SOUND
        printf("Access type not available for playback: %s\n", snd_strerror(err));
#endif
        return err;
    }
    /* set the sample format */
    err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_U8);
    if (err < 0)
    {
#ifdef DEBUG_SOUND
        printf("Sample format not available for playback: %s\n", snd_strerror(err));
#endif
        return err;
    }
    /* set the count of channels */
    err = snd_pcm_hw_params_set_channels(handle, params, 1);
    if (err < 0)
    {
#ifdef DEBUG_SOUND
        printf("Channels count not available for playbacks: %s\n", snd_strerror(err));
#endif
        return err;
    }
    /* set the stream rate */
    rrate = SOUND_RATE;
    err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
    if (err < 0)
    {
#ifdef DEBUG_SOUND
        printf("Rate %iHz not available for playback: %s\n", rrate, snd_strerror(err));
#endif
        return err;
    }
    if (rrate != SOUND_RATE)
    {
#ifdef DEBUG_SOUND
        printf("Rate doesn't match (requested %iHz, get %iHz)\n", SOUND_RATE, err);
#endif
        return -EINVAL;
    }
    /* set the buffer time */
    err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time, &dir);
    if (err < 0)
    {
#ifdef DEBUG_SOUND
        printf("Unable to set buffer time %i for playback: %s\n", buffer_time, snd_strerror(err));
#endif
        return err;
    }
    err = snd_pcm_hw_params_get_buffer_size(params, &size);
    if (err < 0)
    {
#ifdef DEBUG_SOUND
        printf("Unable to get buffer size for playback: %s\n", snd_strerror(err));
#endif
        return err;
    }
    buffer_size = size;
    /* set the period time */
    err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, &dir);
    if (err < 0)
    {
#ifdef DEBUG_SOUND
        printf("Unable to set period time %i for playback: %s\n", period_time, snd_strerror(err));
#endif
        return err;
    }
    err = snd_pcm_hw_params_get_period_size(params, &size, &dir);
    if (err < 0)
    {
#ifdef DEBUG_SOUND
        printf("Unable to get period size for playback: %s\n", snd_strerror(err));
#endif
        return err;
    }
    period_size = size;
#ifdef DEBUG_SOUND
    printf("Period T:%i S:%i\n", (int)period_time, (int)period_size);
#endif
    /* write the parameters to device */
    err = snd_pcm_hw_params(handle, params);
    if (err < 0)
    {
#ifdef DEBUG_SOUND
        printf("Unable to set hw params for playback: %s\n", snd_strerror(err));
#endif
        return err;
    }
    return 0;
}


int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams)
{
    int err;
    /* get the current swparams */
    err = snd_pcm_sw_params_current(handle, swparams);
    if (err < 0)
    {
#ifdef DEBUG_SOUND
        printf("Unable to determine current swparams for playback: %s\n", snd_strerror(err));
#endif
        return err;
    }

    /* start the transfer when the buffer is almost full: */
    /* (buffer_size / avail_min) * avail_min */
    err = snd_pcm_sw_params_set_start_threshold(handle, swparams, /*(buffer_size / period_size) * */ period_size);
    if (err < 0)
    {
#ifdef DEBUG_SOUND
        printf("Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
#endif
        return err;
    }

    /* allow the transfer when at least period_size samples can be processed */
    /* or disable this mechanism when period event is enabled (aka interrupt like style processing) */
    err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_size);
    if (err < 0)
    {
#ifdef DEBUG_SOUND
        printf("Unable to set avail min for playback: %s\n", snd_strerror(err));
#endif
        return err;
    }
    /* write the parameters to the playback device */
    err = snd_pcm_sw_params(handle, swparams);
    if (err < 0)
    {
#ifdef DEBUG_SOUND
        printf("Unable to set sw params for playback: %s\n", snd_strerror(err));
#endif
        return err;
    }
    return 0;
}

