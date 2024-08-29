/*
Thanks Gruia for implementing AVSpeech.
*/
#pragma once
#include "../Include/SRAL.h"
#include "Engine.h"
#include <string>
class object;

class AVSpeech : public Engine {
public:
	bool Speak(const char* text, bool interrupt)override;
	bool Braille(const char* text)override { return false; }
	bool StopSpeech()override;
	int GetNumber()override {
		return ENGINE_AV_SPEECH;
	}
	bool GetActive()override;
	bool Initialize()override;
	bool Uninitialize()override;
	int GetFeatures()override {
		return SUPPORTS_SPEECH | SUPPORTS_SPEECH_RATE | SUPPORTS_SPEECH_VOLUME | SUPPORTS_SELECT_VOICE;
	}
	void SetVolume(uint64_t value)override;
	uint64_t GetVolume()override;
	void SetRate(uint64_t value)override;
	uint64_t GetRate()override;
	uint64_t GetVoiceCount()override;
	const char* GetVoiceName(uint64_t index)override;
	bool SetVoice(uint64_t index)override;
private:
	object* obj;
};