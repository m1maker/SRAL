#ifndef JAWS_H_
#define JAWS_H_
#pragma once
#include "../Dep/fsapi.h"
#include "../Include/SRAL.h"
#include "Engine.h"
#include <windows.h>
class Jaws : public Engine {
public:
	bool Speak(const char* text, bool interrupt)override;
	bool SpeakSsml(const char* ssml, bool interrupt)override {
		return false;
	}
	void* SpeakToMemory(const char* text, uint64_t* buffer_size, int*channels, int* sample_rate, int* bits_per_sample)override {
		return nullptr;
	}
	bool SetParameter(int param, void* value)override {
		return false;
	}

	void* GetParameter(int param) override {
		return nullptr;
	}

	bool Braille(const char* text)override;
	bool StopSpeech()override;
	bool PauseSpeech()override { return false; }
	bool ResumeSpeech()override { return false; }
	int GetNumber()override {
		return ENGINE_JAWS;
	}
	bool GetActive()override;
	bool Initialize()override;
	bool Uninitialize()override;
	int GetFeatures()override {
		return SUPPORTS_SPEECH | SUPPORTS_BRAILLE;
	}
	void SetVolume(uint64_t)override { return; }
	uint64_t GetVolume()override { return 0; }
	void SetRate(uint64_t)override { return; }
	uint64_t GetRate()override { return 0; }
	uint64_t GetVoiceCount()override {
		return 0;
	}
	const char* GetVoiceName(uint64_t index)override {
		return nullptr;
	}
	bool SetVoice(uint64_t index)override {
		return false;
	}
	int GetKeyFlags()override {
		return HANDLE_NONE;
	}

private:
	IJawsApi* JawsAPI = nullptr;
};
#endif