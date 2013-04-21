//======================================================================================
// speak.cpp - Some quick and dirty speach functions for my phoenix code 
//    I have this split off in my own static library so I can use it in my other
//    Raspberry Pi programs. 
// 
//======================================================================================
#include <string.h>
#include <malloc.h>
#include "speak.h"
#include <espeak/speak_lib.h>
#include "ArduinoDefs.h"
#include "SDRFunctions.h"

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

static const char g_szDefaultVoice[] = {"default"};


extern void _EndSpeak(void);

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
RobotSpeak::RobotSpeak()
{

    _fSpeakInit = false;
    _bIDSDRSpeak = 0xff;

    _pszVoiceFileName = NULL; // Do we have a voice file name?
    _pszVoiceLanguage = NULL; // Default to not specified.
    _bVoiceGender = 0;        // What gender voice should w
}

//-----------------------------------------------------------------------------
// InitSpeak - This function initializes the speech using the current file name
//  or parameters...
//-----------------------------------------------------------------------------
void RobotSpeak::SetVoiceByName(const char *pszVoiceName)
{
    // This overrides all settings
    _pszVoiceFileName = pszVoiceName;
}    

//-----------------------------------------------------------------------------
// InitSpeak - This function initializes the speech using the current file name
//  or parameters...
//-----------------------------------------------------------------------------
// pszLanguage is something like "en" or "en-us"
// gender: 0-none, 1-mail, 2-female
void RobotSpeak::SetVoiceByProperties(const char *pszLanguage, unsigned char gender) 
{
    _pszVoiceLanguage = pszLanguage;
    _bVoiceGender = gender;

}

//-----------------------------------------------------------------------------
// InitSpeak - This function initializes the speech using the current file name
//  or parameters...
//-----------------------------------------------------------------------------
void RobotSpeak::InitSpeak()
{
    espeak_ERROR err = EE_NOT_FOUND; // something other than OK...

    // See if this is the first time through if so register our release function.
    if (_bIDSDRSpeak == 0xff) 
        _bIDSDRSpeak = SDRRegisterReleaseFunction(&_EndSpeak);
        
    // Tell our quick and dirty Sound registry stuff, that we want to be the current one...    
    SDRSetCurrent(_bIDSDRSpeak);
    
    esoutput = AUDIO_OUTPUT_PLAYBACK;
    espeak_Initialize(esoutput, Buflength, path, Options );

    // First see if someone registered a voice file name
    if (_pszVoiceFileName) 
    {
        err = espeak_SetVoiceByName(_pszVoiceFileName);
        if (err != EE_OK) 
            printf("Speak SetVoiceByProp failed %d\n", err);
    }    

    else if (_pszVoiceLanguage)
    {
        espeak_VOICE voice;
        voice.name = NULL;
        voice.languages = _pszVoiceLanguage;
        voice.identifier= NULL;
        voice.age = 0;
        voice.gender = _bVoiceGender;
        voice.variant = 0;
        err = espeak_SetVoiceByProperties(&voice);
        if (err != EE_OK) 
            printf("Speak SetVoiceByProp failed %d\n", err);
    }

    if (err != EE_OK)
        espeak_SetVoiceByName(g_szDefaultVoice);

    _fSpeakInit = true;
}

//-----------------------------------------------------------------------------
// Data definitions
//-----------------------------------------------------------------------------
void RobotSpeak::Speak(const char *psz, bool fWait)
{

    // Tell msound to release itself if necessary...
    printf("Speak %s\n", psz);

    // See if we need to initialize.
    if (!_fSpeakInit)
        InitSpeak();
    else
    {
        // if it is still playing cancel any active stuff...
        if (espeak_IsPlaying())
            espeak_Cancel();
    }

    if (psz)
    {
        espeak_Synth( psz, strlen(psz)+1, 0, POS_CHARACTER, 0, espeakCHARS_AUTO, &_uSpeakIdentifier, 0 );
        if (fWait)
            espeak_Synchronize();
    }
}

    

//-----------------------------------------------------------------------------
// EndSpeak - terminate our usage of espeak
//-----------------------------------------------------------------------------
void RobotSpeak::EndSpeak(void)
{
    if (_fSpeakInit)
    {
        espeak_Terminate();
        _fSpeakInit = false;
    }
}

// We need a non member to do the main work
void _EndSpeak(void) {
    Speak.EndSpeak();
}

//-----------------------------------------------------------------------------
// Define our global object
//-----------------------------------------------------------------------------
RobotSpeak Speak;



