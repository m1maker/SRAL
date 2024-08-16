#ifndef NVDA_H_
#define NVDA_H_
#ifdef _WIN32
#pragma once
#include "../Include/SRAL.h"
#include "Engine.h"
#include <Windows.h>
class NVDA : public Engine {
public:
	bool Speak(const char* text, bool interrupt);
	bool Braille(const char* text);
	bool StopSpeech();
	int GetNumber() {
		return ENGINE_NVDA;
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
	HINSTANCE lib = nullptr;
	typedef error_status_t(__stdcall* NVDAController_speakText)(const wchar_t*);
	typedef error_status_t(__stdcall* NVDAController_brailleMessage)(const wchar_t*);
	typedef error_status_t(__stdcall* NVDAController_cancelSpeech)();
	typedef error_status_t(__stdcall* NVDAController_testIfRunning)();

	NVDAController_speakText nvdaController_speakText = nullptr;
	NVDAController_brailleMessage nvdaController_brailleMessage = nullptr;
	NVDAController_cancelSpeech nvdaController_cancelSpeech = nullptr;
	NVDAController_testIfRunning nvdaController_testIfRunning = nullptr;
};
#endif
#endif