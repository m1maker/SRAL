#ifndef JAWS_H_
#define JAWS_H_
#ifdef _WIN32
#pragma once
#include "../Dep/fsapi.h"
#include "../Include/SRAL.h"
#include "ScreenReader.h"
#include <Windows.h>
class Jaws : public ScreenReader {
public:
	bool Speak(const char* text, bool interrupt);
	bool Braille(const char* text);
	bool StopSpeech();
	int GetNumber() {
		return SCREEN_READER_JAWS;
	}
	bool GetActive();
	bool Initialize();
	bool Uninitialize();
	int GetFeatures() {
		return SUPPORTS_SPEECH | SUPPORTS_BRAILLE;
	}
	void SetVolume(uint64_t) { return; }
	uint64_t GetVolume() { return 0; }
	void SetRate(uint64_t) { return; }
	uint64_t GetRate() { return 0; }
private:
	IJawsApi* JawsAPI = nullptr;
};
#endif
#endif