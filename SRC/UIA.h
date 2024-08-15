#ifndef UIA_H_
#define UIA_H_
#pragma once
#include "../Include/SRAL.h"
#include "ScreenReader.h"
class UIA : public ScreenReader {
public:
	bool Speak(const char* text, bool interrupt);
	bool Braille(const char* text) { return false; }
	bool StopSpeech();
	int GetNumber() {
		return SCREEN_READER_UIA;
	}
	bool GetActive();
	bool Initialize();
	bool Uninitialize();
	int GetFeatures() {
		return SUPPORTS_SPEECH;
	}
	void SetVolume(uint64_t) { return; }
	uint64_t GetVolume() { return 0; }
	void SetRate(uint64_t) { return; }
	uint64_t GetRate() { return 0; }
private:
	int val = 0;
};
#endif