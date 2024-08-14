#ifndef SAPI_H_
#define SAPI_H_
#pragma once
#define BLASTSPEAK_IMPLEMENTATION
#include "../Dep/blastspeak.h"
#include "../Dep/miniaudio.h"
#include "../Include/SRAL.h"
#include "ScreenReader.h"
class SAPI : public ScreenReader {
public:
	bool Speak(const char* text, bool interrupt);
	bool Braille(const char* text) { return false; }
	bool StopSpeech();
	int GetNumber() {
		return SCREEN_READER_SAPI;
	}
	bool GetActive();
	bool Initialize();
	bool Uninitialize();
private:
	blastspeak* instance = nullptr;
	ma_engine m_audioEngine;
	ma_sound m_sound;
	ma_audio_buffer m_buffer;
	bool m_bufferInitialized = false;
	bool m_soundInitialized = false;
};

#endif