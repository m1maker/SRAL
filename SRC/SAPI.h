#ifndef SAPI_H_
#define SAPI_H_
#ifdef _WIN32
#pragma once
#define BLASTSPEAK_IMPLEMENTATION
#include "../Dep/blastspeak.h"
#include "../Dep/miniaudio.h"
#include "../Include/SRAL.h"
#include "Engine.h"
class SAPI : public Engine {
public:
	bool Speak(const char* text, bool interrupt);
	bool Braille(const char* text) { return false; }
	bool StopSpeech();
	int GetNumber() {
		return ENGINE_SAPI;
	}
	bool GetActive();
	bool Initialize();
	bool Uninitialize();
	int GetFeatures() {
		return SUPPORTS_SPEECH | SUPPORTS_SPEECH_RATE | SUPPORTS_SPEECH_VOLUME;
	}
	void SetVolume(uint64_t value);
	uint64_t GetVolume();
	void SetRate(uint64_t value);
	uint64_t GetRate();
private:
	blastspeak* instance = nullptr;
	ma_engine m_audioEngine;
	ma_sound m_sound;
	ma_audio_buffer m_buffer;
	bool m_bufferInitialized = false;
	bool m_soundInitialized = false;
};
#endif
#endif