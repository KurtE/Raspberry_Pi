/*
  speak.h - Speech header file for support for Kurt's robots
  By Kurt Eckhardt

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

//======================================================================================
// speak.h - Some quick and dirty speach functions for my phoenix code
//======================================================================================
#ifndef _SPEAK_H_
#define _SPEAK_H_

class RobotSpeak
{
    public:
        RobotSpeak(void);                        // Constructor
        void Speak(const char *psz, bool fWait); // A string to output to the speaker
        void EndSpeak(void);                     // End voice output 

        void SetVoiceByName(const char *pszVoiceName); // specific voice file name

        // pszLanguage is something like "en" or "en-us"
        // gender: 0-none, 1-mail, 2-female
        void SetVoiceByProperties(const char *pszLanguage, unsigned char gender);

    private:
        bool             _fSpeakInit;
        unsigned int      _uSpeakIdentifier;     // Unique identifier
        unsigned char    _bIDSDRSpeak;

        const char       *_pszVoiceFileName;     // Do we have a voice file name?
        const char       *_pszVoiceLanguage;     // Default to not specified.
        unsigned char    _bVoiceGender;          // What gender voice should w
        // private members
        void InitSpeak(void);


};

extern RobotSpeak Speak;
#endif
