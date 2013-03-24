/* 
   TITLE: Simple C/C++ Program showing use of speak_lib.h 
   AUTHOR:Dhananjay Singh
   LICENSE: GPLv2
*/
#include <string.h>
#include <malloc.h>
#include <speak_lib.h>



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
/* 
    Voice: Refer to speak_lib.h 
*/

char text[20] = {"Hello World!"};
unsigned int Size,position=0, end_position=0, flags=espeakCHARS_AUTO, *unique_identifier;

/*    
   text: The text to be spoken, terminated by a zero character. It may be either 8-bit characters,
      wide characters (wchar_t), or UTF8 encoding.  Which of these is determined by the "flags"
      parameter.

   Size: Equal to (or greatrer than) the size of the text data, in bytes.  This is used in order
      to allocate internal storage space for the text.  This value is not used for
      AUDIO_OUTPUT_SYNCHRONOUS mode.

   position:  The position in the text where speaking starts. Zero indicates speak from the
      start of the text.

   position_type:  Determines whether "position" is a number of characters, words, or sentences.
      Values: 

   end_position:  If set, this gives a character position at which speaking will stop.  A value
      of zero indicates no end position.

   flags:  These may be OR'd together:
      Type of character codes, one of:
         espeakCHARS_UTF8     UTF8 encoding
         espeakCHARS_8BIT     The 8 bit ISO-8859 character set for the particular language.
         espeakCHARS_AUTO     8 bit or UTF8  (this is the default)
         espeakCHARS_WCHAR    Wide characters (wchar_t)

      espeakSSML   Elements within < > are treated as SSML elements, or if not recognised are ignored.

      espeakPHONEMES  Text within [[ ]] is treated as phonemes codes (in espeak's Hirshenbaum encoding).

      espeakENDPAUSE  If set then a sentence pause is added at the end of the text.  If not set then
         this pause is suppressed.

   unique_identifier: message identifier; helpful for identifying later 
     data supplied to the callback.

   user_data: pointer which will be passed to the callback function.

   espeak_Synth() Returns: EE_OK: operation achieved 
                           EE_BUFFER_FULL: the command can not be buffered; 
                           you may try after a while to call the function again.
                    	   EE_INTERNAL_ERROR.
*/





int main(int argc, char* argv[] ) 
{
    output = AUDIO_OUTPUT_PLAYBACK;
    int I, Run = 1, L;    
    espeak_Initialize(output, Buflength, path, Options ); 
    espeak_SetVoiceByName(Voice);
    Size = strlen(text)+1;    
    printf("Saying  '%s'",text);
    espeak_Synth( text, Size, position, position_type, end_position, flags,
    unique_identifier, user_data );
    espeak_Synchronize( );
    printf("\n:Done\n"); 
    return 0;
}
