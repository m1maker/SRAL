#ifndef SAPI_H_
#define SAPI_H_
#pragma once
#define BLASTSPEAK_IMPLEMENTATION
#include "../Dep/blastspeak.h"
#include "../Dep/wasapi.h"
#include "../Include/SRAL.h"
#include "Engine.h"

class SAPI : public Engine {
public:
	bool Speak(const char* text, bool interrupt)override;
	bool SpeakSsml(const char* ssml, bool interrupt)override {
		return false;
	}
	void* SpeakToMemory(const char* text, uint64_t* buffer_size, int*channels, int* sample_rate, int* bits_per_sample)override;
	bool SetParameter(int param, const void* value)override;
	bool GetParameter(int param, void* value) override;

	bool Braille(const char* text)override { return false; }
	bool StopSpeech()override;
	bool PauseSpeech()override;
	bool ResumeSpeech()override;
	int GetNumber()override {
		return ENGINE_SAPI;
	}
	bool GetActive()override;
	bool Initialize()override;
	bool Uninitialize()override;
	int GetFeatures()override {
		return SUPPORTS_SPEECH | SUPPORTS_SPEECH_RATE | SUPPORTS_SPEECH_VOLUME | SUPPORTS_SELECT_VOICE | SUPPORTS_PAUSE_SPEECH | SUPPORTS_SPEAK_TO_MEMORY;
	}
	void SetVolume(uint64_t value)override;
	uint64_t GetVolume()override;
	void SetRate(uint64_t value)override;
	uint64_t GetRate()override;
	uint64_t GetVoiceCount()override;
	const char* GetVoiceName(uint64_t index)override;
	bool SetVoice(uint64_t index)override;
	int GetKeyFlags()override {
		return HANDLE_INTERRUPT | HANDLE_PAUSE_RESUME;
	}

private:
	blastspeak* instance = nullptr;
	WAVEFORMATEX wfx;
	int trimThreshold = 20;
	int voiceIndex = 0;
};
#endif