//======================================================================================
// speak.cpp - Some quick and dirty speach functions for my phoenix code
//======================================================================================
#include <string.h>
#include <malloc.h>
#include <espeak/speak_lib.h>
#include "Hex_Cfg.h"
#include "Phoenix.h"
#ifdef OPT_ESPEAK

//-----------------------------------------------------------------------------
// Data definitions
//-----------------------------------------------------------------------------
espeak_POSITION_TYPE    position_type;
espeak_AUDIO_OUTPUT     esoutput;
char                    *path=NULL;
int Buflength = 500, Options=0;
void*                   user_data;
//t_espeak_callback 		*SynthCallback;
espeak_PARAMETER        Parm;

#if defined(ESPEAK_VOICENAME)
static const char g_szVoice[] = {ESPEAK_VOICENAME};

#elif defined(ESPEAK_LANGUAGE)
#ifndef ESPEAK_GENDER
#define ESPEAK_GENDER 0	// don't care
#endif
#ifndef ESPEAK_AGE
#define ESPEAK_AGE 0		// don't care
#endif
#ifndef ESPEAK_VARIANT
#define ESPEAK_VARIANT 0	// best fit
#endif
static const char g_szSpeakLang[]= {ESPEAK_LANGUAGE};
//static const char g_szSpeakLang[]= {"en-us"};
#endif
static const char g_szDefaultVoice[] = {"default"};

unsigned int unique_identifier;

bool                    g_fSpeakInit = false;

//-----------------------------------------------------------------------------
// Data definitions
//-----------------------------------------------------------------------------
void InitSpeak()
{
    espeak_ERROR err;
    
    esoutput = AUDIO_OUTPUT_PLAYBACK;
    espeak_Initialize(esoutput, Buflength, path, Options );
#if defined(ESPEAK_VOICENAME)
    err = espeak_SetVoiceByName(g_szVoice);
    if (err != EE_OK) 
    {
        printf("Speak SetVoiceByProp failed %d\n", err);
        espeak_SetVoiceByName(g_szDefaultVoice);
    }

#elif defined(ESPEAK_LANGUAGE)
    espeak_VOICE voice;
    voice.name = NULL;
    voice.languages = g_szSpeakLang;
    voice.identifier= NULL;
    voice.age = ESPEAK_AGE;
    voice.gender = ESPEAK_GENDER;
    voice.variant = ESPEAK_VARIANT;
    err = espeak_SetVoiceByProperties(&voice);
    if (err != EE_OK) 
    {
        printf("Speak SetVoiceByProp failed %d\n", err);
        espeak_SetVoiceByName(g_szDefaultVoice);
    }
#else    
    espeak_SetVoiceByName(g_szDefaultVoice);
#endif
    g_fSpeakInit = true;
}

//-----------------------------------------------------------------------------
// Data definitions
//-----------------------------------------------------------------------------
void Speak(const char *psz, bool fWait)
{

    // Tell msound to release itself if necessary...
    printf("Speak %s\n", psz);
#ifdef OPT_PCMSOUND
    MSound(0);
#endif
    // See if we need to initialize.
    if (!g_fSpeakInit)
        InitSpeak();
    else
    {
        // if it is still playing cancel any active stuff...
        if (espeak_IsPlaying())
            espeak_Cancel();
    }

    if (psz)
    {
        espeak_Synth( psz, strlen(psz)+1, 0, POS_CHARACTER, 0, espeakCHARS_AUTO, &unique_identifier, 0 );
        if (fWait)
            espeak_Synchronize();
    }
}

    

//-----------------------------------------------------------------------------
// EndSpeak - terminate our usage of espeak
//-----------------------------------------------------------------------------
void EndSpeak(void)
{
    if (g_fSpeakInit)
    {
        espeak_Terminate();
        g_fSpeakInit = false;
    }
}


#else
void Speak(char *psz, bool fWait)
{
}


void EndSpeak(void)
{
}
#endif
