//======================================================================================
// speak.cpp - Some quick and dirty speach functions for my phoenix code
//======================================================================================
#ifdef OPT_ESPEAK
#include <string.h>
#include <malloc.h>
#include <espeak/speak_lib.h>
#include "Hex_Cfg.h"

//-----------------------------------------------------------------------------
// Data definitions
//-----------------------------------------------------------------------------
espeak_POSITION_TYPE 	position_type;
espeak_AUDIO_OUTPUT		output;
char 					*path=NULL;
int Buflength = 500, Options=0;
void* 					user_data;
//t_espeak_callback 		*SynthCallback;
espeak_PARAMETER 		Parm;

char Voice[] = {"default"};
unsigned int unique_identifier;

bool					g_fSpeakInit = false;

//-----------------------------------------------------------------------------
// Data definitions
//-----------------------------------------------------------------------------
void Speak(char *psz, bool fWait) {
	
	// See if we need to initialize.
	if (!g_fSpeakInit) {
		output = AUDIO_OUTPUT_PLAYBACK;
		espeak_Initialize(output, Buflength, path, Options ); 
		espeak_SetVoiceByName(Voice);
		g_fSpeakInit = true;
	}
	else {
		// if it is still playing cancel any active stuff...
		if (espeak_IsPlaying())
			espeak_Cancel();
	}		

	if (psz) {
		espeak_Synth( psz, strlen(psz)+1, 0, 0, 0, espeakCHARS_AUTO, &unique_identifier, 0 );
		if (fWait)
			espeak_Synchronize();
	}
}

void EndSpeak(void) {
	if (g_fSpeakInit) {
		espeak_Terminate();
		g_fSpeakInit = false;
	}
}
#else
void Speak(char *psz, bool fWait) {
}
void EndSpeak(void) {
}
#endif
