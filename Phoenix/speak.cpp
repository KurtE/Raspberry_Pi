#ifdef TRY_FESTIVAL
#include <stdio.h>
#include <festival/festival.h>

void SaySomething (char *psz) {
   EST_Wave wave;
    int heap_size = 210000;  // default scheme heap size
    int load_init_files = 1; // we want the festival init files loaded

    festival_initialize(load_init_files,heap_size);

    // Say simple file
    //festival_say_file("/etc/motd");

    festival_eval_command("(voice_ked_diphone)");
    // Say some text;
    festival_say_text("hello world");

    // Convert to a waveform
    festival_text_to_wave(psz,wave);
    wave.save("/tmp/wave.wav","riff");

    // festival_say_file puts the system in async mode so we better
    // wait for the spooler to reach the last waveform before exiting
    // This isn't necessary if only festival_say_text is being used (and
    // your own wave playing stuff)
    festival_wait_for_spooler();
}
#else
#include <string.h>
#include <malloc.h>
#include <espeak/speak_lib.h>


espeak_POSITION_TYPE position_type;
espeak_AUDIO_OUTPUT output;
char *path=NULL;
int Buflength = 500, Options=0;
void* user_data;
t_espeak_callback *SynthCallback;
espeak_PARAMETER Parm;

/*
   FROM speak_lib.h :

   output: the audio data can either be played by eSpeak or passed back by the SynthCallback function.

   Buflength:  The length in mS of sound buffers passed to the SynthCallback function.

   options: bit 0: 1=allow espeakEVENT_PHONEME events.

   path: The directory which contains the espeak-data directory, or NULL for the default location.

   espeak_Initialize() Returns: sample rate in Hz, or -1 (EE_INTERNAL_ERROR).
*/

char Voice[] = {"default"};
unsigned int Size,position=0, end_position=0, flags=espeakCHARS_AUTO, *unique_identifier;

void SaySomething (char *psz) {
    output = AUDIO_OUTPUT_PLAYBACK;
//    int I, Run = 1, L;    
    espeak_Initialize(output, Buflength, path, Options ); 
    espeak_SetVoiceByName(Voice);
    Size = strlen(psz)+1;    
//    printf("Saying  '%s'",text);
    espeak_Synth( psz, Size, position, position_type, end_position, flags, unique_identifier, user_data );
    espeak_Synchronize( );
//    printf("\n:Done\n"); 
}
#endif


