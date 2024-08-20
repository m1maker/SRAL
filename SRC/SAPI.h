#ifndef SAPI_H_
#define SAPI_H_
#ifdef _WIN32
#pragma once
#define BLASTSPEAK_IMPLEMENTATION
#include "../Dep/blastspeak.h"
#include "../Include/SRAL.h"
#include "Engine.h"
class SAPI : public Engine {
public:
	bool Speak(const char* text, bool interrupt)override;
	bool Braille(const char* text)override { return false; }
	bool StopSpeech()override;
	int GetNumber()override {
		return ENGINE_SAPI;
	}
	bool GetActive()override;
	bool Initialize()override;
	bool Uninitialize()override;
	int GetFeatures()override {
		return SUPPORTS_SPEECH | SUPPORTS_SPEECH_RATE | SUPPORTS_SPEECH_VOLUME;
	}
	void SetVolume(uint64_t value)override;
	uint64_t GetVolume()override;
	void SetRate(uint64_t value)override;
	uint64_t GetRate()override;
private:
	blastspeak* instance = nullptr;
	WAVEFORMATEX wfx;
	WAVEHDR wh;
	HWAVEOUT hWaveOut;
};
#endif
#endif