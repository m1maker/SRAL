#ifndef JAWS_H_
#define JAWS_H_
#ifdef _WIN32
#pragma once
#include "../Dep/fsapi.h"
#include "../Include/SRAL.h"
#include "Engine.h"
#include <Windows.h>
class Jaws : public Engine {
public:
	bool Speak(const char* text, bool interrupt)override;
	bool Braille(const char* text)override;
	bool StopSpeech()override;
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
private:
	IJawsApi* JawsAPI = nullptr;
};
#endif
#endif