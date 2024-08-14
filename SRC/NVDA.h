#ifndef NVDA_H_
#define NVDA_H_
#pragma once
#include "../Include/SRAL.h"
#include "ScreenReader.h"
#include <Windows.h>
class NVDA : public ScreenReader {
public:
	bool Speak(const char* text, bool interrupt);
	bool Braille(const char* text);
	bool StopSpeech();
	int GetNumber() {
		return SCREEN_READER_NVDA;
	}
	bool GetActive();
	bool Initialize();
	bool Uninitialize();
private:
	HINSTANCE lib;
	typedef error_status_t(__stdcall* NVDAController_speakText)(const wchar_t*);
	typedef error_status_t(__stdcall* NVDAController_brailleMessage)(const wchar_t*);
	typedef error_status_t(__stdcall* NVDAController_cancelSpeech)();
	typedef error_status_t(__stdcall* NVDAController_testIfRunning)();

	NVDAController_speakText nvdaController_speakText;
	NVDAController_brailleMessage nvdaController_brailleMessage;
	NVDAController_cancelSpeech nvdaController_cancelSpeech;
	NVDAController_testIfRunning nvdaController_testIfRunning;

};

#endif